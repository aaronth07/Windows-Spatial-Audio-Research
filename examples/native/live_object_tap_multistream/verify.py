"""Offline validation of multistream source PCM and object lifecycle metadata."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import struct
import subprocess


def f32(value):
    return struct.unpack('<f', struct.pack('<f', value))[0]


def validate(folder, capture=None, pcm=None):
    metadata = capture if capture is not None else json.loads((folder / 'capture.json').read_text())
    pcm = pcm if pcm is not None else (folder / 'capture.f32').read_bytes()
    reference = json.loads((folder / 'reference.json').read_text())
    assert pcm == (folder / 'reference.f32').read_bytes(), 'PCM mismatch or truncation'
    assert metadata['rate'] == reference['rate'] == 48000
    assert metadata['coordinate_system'] == reference['coordinate_system']
    assert len(metadata['records']) == len(reference['records']) > 0
    totals, last_updates, ids, eos_counts = {}, {}, {}, {}
    next_sample = 0
    for actual, expected in zip(metadata['records'], reference['records']):
        assert {k: v for k, v in actual.items() if k != 'object_id'} == {k: v for k, v in expected.items() if k != 'object_id'}, 'metadata reference mismatch'
        stream, source, generation = actual['stream_id'], actual['source_index'], actual['generation']
        assert stream in (1, 2) and source in (0, 1)
        assert generation in ((0, 1) if source == 0 else (0,))
        key, source_key = (stream, source, generation), (stream, source)
        assert actual['object_id'] > 0
        assert key not in ids or ids[key] == actual['object_id'], 'ID changed within lifetime'
        ids[key] = actual['object_id']
        update = actual['update']
        assert update > 0 and update >= last_updates.get(stream, 0), 'per-stream update order'
        last_updates[stream] = update
        frames, valid = actual['frames'], actual['valid_frames']
        assert 0 < valid <= frames <= 8192 and actual['sample_offset'] == next_sample
        assert 4 * (next_sample + frames) <= len(pcm)
        values = struct.unpack_from('<{}f'.format(frames), pcm, 4 * next_sample)
        offset = totals.get(source_key, 0)
        generated = []
        for index in range(valid):
            value = (((offset + index + 17 * source + 100003 * (stream - 1)) * 1664525) + 1013904223) & 0xffffffff
            generated.append(f32(((value >> 9) / 8388608.0 - .5) * f32(.02)))
        assert tuple(generated) == values[:valid], 'independent PCM mismatch'
        assert all(x == 0 for x in values[valid:]), 'nonzero EOS padding'
        x = f32(f32(f32(.75 if source else -.75) + f32(f32(.05) * (stream - 1))) + f32(f32(.1) * f32(offset / 48000.0)))
        assert [f32(x) for x in actual['position']] == [x, f32(.15 if source else -.15), -1.0 - .25 * (stream - 1)]
        assert f32(actual['volume']) == f32(.00075 if source else .0005)
        assert actual['type'] == 1 and actual['forward_end_hresult'] == 0
        assert generation == (int(offset >= 24000) if source == 0 else 0)
        totals[source_key] = offset + valid
        expected_eos = totals[source_key] == (24000 if source == 0 and generation == 0 else 48000)
        assert bool(actual['eos']) == expected_eos
        eos_counts[key] = eos_counts.get(key, 0) + actual['eos']
        next_sample += frames
    assert len(totals) == 4 and all(x == 48000 for x in totals.values())
    assert len(ids) == len(set(ids.values())) == 6, 'object lifetime IDs reused'
    assert all(x == 1 for x in eos_counts.values())
    for stream in (1, 2):
        assert ids[(stream, 0, 1)] > ids[(stream, 0, 0)], 'recreated object needs new monotonic ID'
    assert next_sample * 4 == len(pcm)
    result = json.loads((folder / 'result.json').read_text())
    assert result['passed'] and result['unique_object_lifetimes'] == 6 and result['elapsed_seconds'] < 2
    assert result['proxy_references_released'] and result['producer_imports_restored']
    assert result['unknown_interface_passthroughs'] == 0
    assert result['begin_calls'] == result['forwarded_end_calls']
    assert result['records_copied_before_forward_end'] == result['buffer_pointers_cleared_after_end'] == len(metadata['records'])
    assert len(result['streams']) == 2
    for stream in result['streams']:
        assert stream['eos_inactive_objects'] == 2 and stream['recreated_objects'] == 1
        assert stream['producer_hresult'] == stream['capture_hresult'] == stream['stop_hresult'] == stream['reset_hresult'] == 0
    return {'records': len(metadata['records']), 'float_samples': next_sample,
            'valid_frames_per_source': {str(k): v for k, v in totals.items()},
            'lifetime_ids': {str(k): v for k, v in ids.items()}}


def hashes(folder):
    return {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in folder.iterdir() if p.is_file()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--capture', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(exist_ok=False)
    here, capture = Path(__file__).resolve().parent, args.capture.resolve()
    before = hashes(capture)
    summary, cases = validate(capture), []
    native = subprocess.run([str(here / 'offline_tests.exe'), str(here / 'producer.dll')], capture_output=True, text=True)
    assert native.returncode == 0, native.stdout + native.stderr
    native_result = json.loads(native.stdout)
    assert native_result['passed'] and native_result['windows_audio_streams_started'] == 0
    for name, argv in [
        ('existing_output', [str(here / 'producer.dll'), str(capture)]),
        ('missing_dll', [str(here / 'missing_producer.dll'), str(args.output / 'missing_dll')]),
        ('invalid_dll', [str(here / 'protocol.h'), str(args.output / 'invalid_dll')]),
        ('missing_arguments', []),
    ]:
        completed = subprocess.run([str(here / 'tap_host.exe')] + argv, capture_output=True, text=True)
        assert completed.returncode != 0, name
        cases.append({'name': name, 'returncode': completed.returncode, 'diagnostic': completed.stderr.strip()})
    original, original_pcm = json.loads((capture / 'capture.json').read_text()), (capture / 'capture.f32').read_bytes()
    mutations = []
    duplicate = copy.deepcopy(original)
    first = duplicate['records'][0]['object_id']
    for record in duplicate['records']:
        if record['stream_id'] == 1 and record['source_index'] == 0 and record['generation'] == 1:
            record['object_id'] = first
    mutations.append(('reused_lifetime_id', duplicate, original_pcm))
    wrong_stream = copy.deepcopy(original); wrong_stream['records'][-1]['stream_id'] = 1
    mutations.append(('cross_stream_metadata', wrong_stream, original_pcm))
    bad_offset = copy.deepcopy(original); bad_offset['records'][0]['sample_offset'] = 2 ** 63
    mutations.append(('invalid_sample_offset', bad_offset, original_pcm))
    bad_eos = copy.deepcopy(original); bad_eos['records'][-1]['valid_frames'] += 1
    mutations.append(('invalid_eos_extent', bad_eos, original_pcm))
    mutations.append(('truncated_pcm', original, original_pcm[:-4]))
    damaged = bytearray(original_pcm); damaged[0] ^= 1
    mutations.append(('corrupted_pcm', original, bytes(damaged)))
    for name, metadata, samples in mutations:
        rejected = False
        try:
            validate(capture, metadata, samples)
        except (AssertionError, ValueError, struct.error):
            rejected = True
        assert rejected, name
        cases.append({'name': name, 'rejected': True})
    assert before == hashes(capture), 'retained capture changed'
    result = {'passed': True, 'native_offline_cases': native_result['offline_cases'], 'additional_cases': len(cases),
              'independent_reference': summary, 'capture_files_unchanged': True,
              'spatial_streams_started_by_verifier': 0, 'cases': cases}
    (args.output / 'result.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
