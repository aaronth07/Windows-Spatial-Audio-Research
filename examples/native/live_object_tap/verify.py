"""Offline checks only: does not start a spatial stream or play audio."""
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
    capture = capture if capture is not None else json.loads((folder / 'capture.json').read_text())
    pcm = pcm if pcm is not None else (folder / 'capture.f32').read_bytes()
    reference = json.loads((folder / 'reference.json').read_text())
    assert pcm == (folder / 'reference.f32').read_bytes(), 'PCM reference mismatch or truncation'
    assert capture['rate'] == 48000 and capture['coordinate_system'] == reference['coordinate_system']
    assert len(capture['records']) == len(reference['records']) > 0
    counts, ids, eos = [0, 0], {}, [0, 0]
    next_sample, last_update = 0, 0
    for actual, expected in zip(capture['records'], reference['records']):
        assert {k: v for k, v in actual.items() if k != 'id'} == {k: v for k, v in expected.items() if k != 'id'}, 'metadata mismatch'
        source = actual['source_index']
        assert source in (0, 1) and actual['id'] > 0
        assert source not in ids or ids[source] == actual['id'], 'unstable ID'
        ids[source] = actual['id']
        assert actual['update'] >= last_update and actual['update'] > 0
        last_update = actual['update']
        frames, valid = actual['frames'], actual['valid_frames']
        assert 0 < valid <= frames <= 8192
        assert actual['sample_offset'] == next_sample
        assert 4 * (next_sample + frames) <= len(pcm)
        values = struct.unpack_from('<{}f'.format(frames), pcm, 4 * next_sample)
        expected_pcm = []
        for index in range(valid):
            value = (((counts[source] + index + 17 * source) * 1664525) + 1013904223) & 0xffffffff
            expected_pcm.append(f32(((value >> 9) / 8388608.0 - .5) * f32(.02)))
        assert tuple(expected_pcm) == values[:valid], 'independent source PCM mismatch'
        assert all(value == 0 for value in values[valid:]), 'nonzero EOS padding'
        x = f32(f32(.75 if source else -.75) + f32(f32(.1) * f32(counts[source] / 48000.0)))
        assert [f32(x) for x in actual['position']] == [x, f32(.15 if source else -.15), -1.0]
        assert f32(actual['volume']) == f32(.00075 if source else .0005)
        assert actual['forward_end_hresult'] == 0 and actual['type'] == 1
        counts[source] += valid
        assert bool(actual['eos']) == (counts[source] == 48000)
        eos[source] += actual['eos']
        next_sample += frames
    assert counts == [48000, 48000] and eos == [1, 1]
    assert ids[0] < ids[1] and next_sample * 4 == len(pcm)
    result = json.loads((folder / 'result.json').read_text())
    assert result['passed'] and result['eos_inactive_objects'] == 2
    assert result['stop_hresult'] == result['reset_hresult'] == 0
    assert result['begin_calls'] == result['forwarded_end_calls']
    assert result['records_copied_before_forward_end'] == len(capture['records'])
    assert result['buffer_pointers_cleared_after_end'] == len(capture['records'])
    assert result['proxy_references_released'] and result['producer_imports_restored']
    assert result['unknown_interface_passthroughs'] == 0
    return {'records': len(capture['records']), 'valid_frames_per_source': counts,
            'raw_pcm_peak': max(abs(x) for x in struct.unpack('<{}f'.format(next_sample), pcm)),
            'source_volume': [.0005, .00075]}


def digest(folder):
    return {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in folder.iterdir() if p.is_file()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--capture', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(exist_ok=False)
    here = Path(__file__).resolve().parent
    capture = args.capture.resolve()
    before = digest(capture)
    summary = validate(capture)
    cases = []
    native = subprocess.run([str(here / 'offline_tests.exe'), str(here / 'producer.dll')], capture_output=True, text=True)
    assert native.returncode == 0, native.stdout + native.stderr
    native_result = json.loads(native.stdout)
    assert native_result['passed'] and native_result['windows_audio_streams_started'] == 0
    cases.append({'name': 'native_boundaries_and_import_restoration', 'result': native_result})
    for name, argv in [
        ('existing_output', [str(here / 'producer.dll'), str(capture)]),
        ('missing_dll', [str(here / 'missing_producer.dll'), str(args.output / 'missing_dll')]),
        ('invalid_dll', [str(here / 'protocol.h'), str(args.output / 'invalid_dll')]),
        ('missing_arguments', []),
    ]:
        completed = subprocess.run([str(here / 'tap_host.exe')] + argv, capture_output=True, text=True)
        assert completed.returncode != 0, name
        cases.append({'name': name, 'returncode': completed.returncode, 'diagnostic': completed.stderr.strip()})
    original = json.loads((capture / 'capture.json').read_text())
    original_pcm = (capture / 'capture.f32').read_bytes()
    mutations = []
    malformed = copy.deepcopy(original); malformed['records'][0]['id'] = malformed['records'][1]['id']
    mutations.append(('duplicate_object_id', malformed, original_pcm))
    malformed = copy.deepcopy(original); malformed['records'][0]['sample_offset'] = 2 ** 63
    mutations.append(('invalid_sample_offset', malformed, original_pcm))
    malformed = copy.deepcopy(original); malformed['records'][-1]['valid_frames'] += 1
    mutations.append(('invalid_eos_extent', malformed, original_pcm))
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
    assert before == digest(capture), 'capture files changed'
    result = {'passed': True, 'capture': str(capture), 'independent_reference': summary,
              'native_offline_cases': native_result['offline_cases'], 'additional_cases': len(cases) - 1,
              'capture_files_unchanged': True, 'spatial_streams_started_by_verifier': 0, 'cases': cases}
    (args.output / 'result.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
