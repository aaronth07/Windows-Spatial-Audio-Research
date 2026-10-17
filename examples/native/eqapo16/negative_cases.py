"""Reject malformed native probe inputs and preserve existing output files."""
import argparse
import os
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import numpy as np

HERE = Path(__file__).resolve().parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    root = args.output.resolve()
    root.mkdir(parents=True, exist_ok=False)
    records = []

    def reject(name, command, outputs, existing=None):
        before = {p: p.read_bytes() for p in (existing or [])}
        run = subprocess.run(list(map(str, command)), capture_output=True, timeout=20)
        assert run.returncode != 0, name
        assert all(p.read_bytes() == data for p, data in before.items()), name
        assert all(not p.exists() for p in outputs if p not in before), name
        records.append({'name': name, 'exit_code': run.returncode,
                        'diagnostic': (run.stdout + run.stderr).decode('utf-8', errors='replace').strip(),
                        'outputs_preserved': True})

    config = root / 'config'; config.mkdir()
    (config / 'config.txt').write_text('Stage: post-mix\nChannel: all\n')
    good = root / 'valid.f32'; np.zeros((256, 16), dtype=np.float32).tofile(str(good))
    nan = root / 'nan.f32'; np.full((256, 16), np.nan, dtype=np.float32).tofile(str(nan))
    short = root / 'short.f32'; short.write_bytes(b'bad')
    partial = root / 'partial.f32'; np.zeros((127, 16), dtype=np.float32).tofile(str(partial))
    changed = root / 'changed.dll'; changed.write_bytes(b'not the pinned binary')
    for name, channel, block, mode, source, dll in [
            ('zero_channels', '0', '128', 'valid', good, HERE/'runtime/EqualizerAPO.dll'),
            ('high_channels', '33', '128', 'valid', good, HERE/'runtime/EqualizerAPO.dll'),
            ('fractional_channels', '16.0', '128', 'valid', good, HERE/'runtime/EqualizerAPO.dll'),
            ('negative_channels', '-1', '128', 'valid', good, HERE/'runtime/EqualizerAPO.dll'),
            ('short_block', '16', '15', 'valid', good, HERE/'runtime/EqualizerAPO.dll'),
            ('invalid_mode', '16', '128', 'typo', good, HERE/'runtime/EqualizerAPO.dll'),
            ('nan_pcm', '16', '128', 'valid', nan, HERE/'runtime/EqualizerAPO.dll'),
            ('unaligned_pcm', '16', '128', 'valid', short, HERE/'runtime/EqualizerAPO.dll'),
            ('partial_quantum', '16', '128', 'valid', partial, HERE/'runtime/EqualizerAPO.dll'),
            ('wrong_dll', '16', '128', 'valid', good, changed)]:
        out, metrics = root/(name+'.f32'), root/(name+'.json')
        reject(name, [HERE/'eqapo_host.exe', dll, config, source, out, metrics, channel, block, mode], [out, metrics])
    for name, shared in [('existing_output', False), ('same_output_paths', True)]:
        out, metrics = root/(name+'.f32'), root/(name+'.json')
        existing=[]
        if shared: metrics = out
        else: out.write_bytes(b'preserve existing output'); existing=[out]
        reject(name, [HERE/'eqapo_host.exe', HERE/'runtime/EqualizerAPO.dll', config, good, out, metrics, 16, 128, 'valid'], [out, metrics], existing)

    positions=np.asarray([[1,1,1],[1,-1,-1],[-1,1,-1],[-1,-1,1]],dtype=np.float32)/np.float32(np.sqrt(3))
    triangles=np.asarray([[0,1,2],[0,1,3],[0,2,3],[1,2,3]],dtype=np.uint32)
    geometry=positions.astype('<f4').tobytes()+triangles.astype('<u4').tobytes()
    metadata_bytes=struct.pack('<4f',1,0,0,1)
    valid=struct.pack('<6I',0x31504256,4,1,1,128,4)+geometry+metadata_bytes+bytes(128*4)
    def mutate(offset, data):
        changed = bytearray(valid); changed[offset:offset+len(data)] = data; return changed
    positions = 24; faces = positions + 4*3*4
    metadata = faces + struct.unpack_from('<I', valid, 20)[0]*3*4
    cases = {
        'truncated': valid[:40], 'trailing': valid+b'x',
        'invalid_magic': mutate(0, struct.pack('<I', 0)),
        'too_many_objects': mutate(8, struct.pack('<I', 129)),
        'invalid_quantum': mutate(16, struct.pack('<I', 2049)),
        'nonunit_speaker': mutate(positions, struct.pack('<f', 30)),
        'nan_speaker': mutate(positions, struct.pack('<f', float('nan'))),
        'invalid_triangle_index': mutate(faces, struct.pack('<I', 4)),
        'degenerate_triangle': mutate(faces, struct.pack('<3I', 0, 0, 0)),
        'zero_position': mutate(metadata, struct.pack('<3f', 0, 0, 0)),
        'negative_gain': mutate(metadata+12, struct.pack('<f', -1)),
        'nonfinite_gain': mutate(metadata+12, struct.pack('<f', float('inf'))),
    }
    for name, content in cases.items():
        source = root/('vbap_'+name+'.bin'); source.write_bytes(content)
        out, metrics = root/('vbap_'+name+'.f32'), root/('vbap_'+name+'.json')
        reject('vbap_'+name, [HERE/'vbap_probe.exe', source, out, metrics], [out, metrics])

    lifetime=struct.pack('<6I',0x32504256,4,2,1,128,4)+geometry+struct.pack('<2Q',1,2)+metadata_bytes*2+bytes(2*128*4)
    lifetime_header=struct.unpack_from('<6I',lifetime)
    offset=24+lifetime_header[1]*12+lifetime_header[5]*12
    for name,field,value in [('missing_lifetime',0,0),('duplicate_lifetime',1,struct.unpack_from('<Q',lifetime,offset)[0])]:
        changed=bytearray(lifetime);struct.pack_into('<Q',changed,offset+field*8,value)
        source=root/(name+'.bin');source.write_bytes(changed)
        out,metrics=root/(name+'.f32'),root/(name+'.json')
        reject(name,[HERE/'vbap_probe.exe',source,out,metrics],[out,metrics])

    # No objects is a valid silent scene, not an input error.
    header = list(struct.unpack_from('<6I', valid)); header[2] = 0
    source = root/'zero_objects.bin'; source.write_bytes(struct.pack('<6I', *header)+valid[24:metadata])
    out, metrics = root/'zero_objects.f32', root/'zero_objects.json'
    run = subprocess.run([str(HERE/'vbap_probe.exe'), str(source), str(out), str(metrics)], capture_output=True, timeout=20)
    assert run.returncode == 0 and np.all(np.fromfile(str(out), dtype=np.float32) == 0)
    records.append({'name': 'zero_objects', 'exit_code': 0, 'silent_output': True})
    result = {'cases': records, 'passed': True, 'executables': {name: hashlib.sha256((HERE/name).read_bytes()).hexdigest() for name in ['eqapo_host.exe', 'vbap_probe.exe']}}
    (root/'result.json').write_text(json.dumps(result, indent=2))
    print(json.dumps({'cases': len(records), 'passed': True}))


if __name__ == '__main__': main()
