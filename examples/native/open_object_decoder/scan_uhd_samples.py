"""Inspect every MP4 DTS-UHD access-unit prefix, without decoding or CRC claims."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
from inspect_fixture import boxes, uhd_prefix


def require(condition, message):
    if not condition: raise ValueError(message)


def sample_offsets(sizes, chunks, entries, file_bytes):
    require(bool(entries) and entries[0][0] == 1, 'Invalid first sample-to-chunk entry')
    require(all(1 <= e[0] <= len(chunks) and e[1] > 0 and e[2] == 1 for e in entries), 'Invalid chunk/description')
    require(all(a[0] < b[0] for a, b in zip(entries, entries[1:])), 'Unordered chunk entries')
    result, sample, entry = [], 0, 0
    for chunk, offset in enumerate(chunks, 1):
        while entry+1 < len(entries) and entries[entry+1][0] <= chunk: entry += 1
        for _ in range(entries[entry][1]):
            require(sample < len(sizes), 'Too many chunk samples')
            size = sizes[sample]
            require(size > 0 and offset >= 0 and offset+size <= file_bytes, 'Sample outside file')
            result.append((offset, size)); offset += size; sample += 1
    require(sample == len(sizes), 'Samples missing from chunks')
    return result


def self_test():
    require(sample_offsets([3, 4, 5], [10, 50], [(1, 2, 1), (2, 1, 1)], 100) == [(10, 3), (13, 4), (50, 5)], 'Known chunk mapping')
    bad = [([3], [10], [(2, 1, 1)], 100), ([3], [10], [(1, 1, 2)], 100),
           ([3], [10], [(1, 0, 1)], 100), ([3], [99], [(1, 1, 1)], 100),
           ([3, 4], [10], [(1, 1, 1)], 100), ([3], [10], [(1, 2, 1)], 100)]
    for args in bad:
        rejected = False
        try: sample_offsets(*args)
        except ValueError: rejected = True
        require(rejected, 'Malformed chunk map accepted')
    return {'passed': True, 'chunk_mapping_cases': 7}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('media', type=Path, nargs='?'); parser.add_argument('output', type=Path, nargs='?')
    parser.add_argument('--self-test', action='store_true'); args = parser.parse_args()
    if args.self_test: print(json.dumps(self_test())); return
    require(args.media is not None and args.output is not None, 'Media and new output required')
    if args.output.exists(): raise FileExistsError(args.output)
    require(0 < args.media.stat().st_size <= 512*1024*1024, 'Media exceeds inspection bound')
    data = args.media.read_bytes()
    def children(kind, start, end): return [(a, b) for k, a, b in boxes(data, start, end) if k == kind]
    tables = []
    for ma, mb in children(b'moov', 0, len(data)):
        for ta, tb in children(b'trak', ma, mb):
            for da, db in children(b'mdia', ta, tb):
                handlers = children(b'hdlr', da, db)
                if not handlers or data[handlers[0][0]+8:handlers[0][0]+12] != b'soun': continue
                for ia, ib in children(b'minf', da, db): tables.extend(children(b'stbl', ia, ib))
    rows = []
    for start, end in tables:
        table = {k: (a, b) for k, a, b in boxes(data, start, end)}
        require(all(k in table for k in (b'stsd', b'stsz', b'stsc')), 'Missing sample table')
        a, b = table[b'stsd']; require(data[a+12:a+16] == b'dtsx', 'Track is not DTS-UHD dtsx')
        a, b = table[b'stsz']; fixed, count = struct.unpack_from('>II', data, a+4)
        require(fixed > 0 or a+12+count*4 <= b, 'Truncated sizes')
        sizes = [fixed]*count if fixed else list(struct.unpack_from('>%dI' % count, data, a+12))
        a, b = table[b'stsc']; entries_count = struct.unpack_from('>I', data, a+4)[0]
        require(a+8+entries_count*12 <= b, 'Truncated sample-to-chunk table')
        entries = [struct.unpack_from('>III', data, a+8+12*i) for i in range(entries_count)]
        wide = b'co64' in table; a, b = table[b'co64' if wide else b'stco']
        chunks_count = struct.unpack_from('>I', data, a+4)[0]; width = 8 if wide else 4
        require(a+8+chunks_count*width <= b, 'Truncated chunk offsets')
        chunks = list(struct.unpack_from('>%d%s' % (chunks_count, 'Q' if wide else 'I'), data, a+8))
        for sample, (offset, size) in enumerate(sample_offsets(sizes, chunks, entries, len(data))):
            packet = data[offset:offset+size]
            row = {'sample': sample, 'offset': offset, 'bytes': size, 'sync': packet[:4].hex()}
            if row['sync'] == '40411bf2': row.update(uhd_prefix(packet))
            rows.append(row)
    require(bool(rows), 'No audio samples')
    result = {'media_sha256': hashlib.sha256(data).hexdigest(), 'samples': len(rows),
              'sync_counts': dict(Counter(r['sync'] for r in rows)),
              'parsed_sync_prefixes': sum('full_channel_based_mix_flag' in r for r in rows),
              'full_channel_based_true': sum(r.get('full_channel_based_mix_flag') is True for r in rows),
              'full_channel_based_false': sum(r.get('full_channel_based_mix_flag') is False for r in rows),
              'records': rows, 'scope': 'Every access-unit start inspected; no full syntax, CRC, internal frame scan or moving-object decoding proof.'}
    args.output.write_text(json.dumps(result, indent=2))
    print(json.dumps({k: v for k, v in result.items() if k != 'records'}))


if __name__ == '__main__': main()
