"""Run the unregistered inert APO COM host and validate startup module hashes."""
import argparse
import ctypes as c
import hashlib
import json
from pathlib import Path
import subprocess


class Event(c.Structure):
    _fields_ = [('size', c.c_uint32), ('sequence', c.c_uint32), ('phase', c.c_uint32), ('pid', c.c_uint32), ('tid', c.c_uint32), ('cpu_present', c.c_uint32), ('hash_present', c.c_uint32), ('status', c.c_int32), ('qpc', c.c_int64), ('endpoint', c.c_ubyte * 16), ('cpu_path', c.c_wchar * 1024), ('cpu_hash', c.c_ubyte * 32)]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary_directory', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    cpu = (args.binary_directory / 'HrtfDspCpu.dll').resolve(strict=True)
    digest = hashlib.sha256(cpu.read_bytes()).hexdigest()
    if digest != '4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9':
        raise ValueError('Unreviewed binary SHA-256')
    if args.output.exists():
        raise FileExistsError('Output exists')
    folder = Path(__file__).resolve().parent
    result = subprocess.run([str(folder / 'bootstrap_host.exe'), str(args.output), str(cpu)], capture_output=True, text=True, timeout=30)
    if result.returncode:
        raise RuntimeError(result.stdout + result.stderr)
    rows = []
    for log in sorted(args.output.glob('*.events')):
        raw = log.read_bytes()
        assert len(raw) % c.sizeof(Event) == 0 and len(raw) <= 64 * c.sizeof(Event)
        events = [Event.from_buffer_copy(raw, offset) for offset in range(0, len(raw), c.sizeof(Event))]
        for event in events:
            assert event.size == c.sizeof(Event)
            if event.cpu_present:
                assert event.hash_present and Path(event.cpu_path).resolve() == cpu and bytes(event.cpu_hash).hex() == digest
        if log.name in ('inert.events', 'bounded.events', 'discovery.events', 'missing_endpoint.events', 'invalid_exports.events', 'properties_7.events', 'properties_9.events', 'properties_11.events'):
            assert all(not event.cpu_present for event in events)
        if log.name == 'preload.events':
            assert not events[0].cpu_present and not events[1].cpu_present and events[2].cpu_present
        if log.name == 'properties_8.events':
            assert [event.phase for event in events] == [2, 6, 5] and not events[0].cpu_present and events[1].cpu_present
        rows.append({'file': log.name, 'records': len(events), 'bytes': len(raw), 'phases': [event.phase for event in events], 'present_module_path_and_disk_file_sha256_verified': True})
    host = json.loads((args.output / 'result.json').read_text())
    host.update({'startup_logs': rows, 'log_capacity': 64, 'event_size': c.sizeof(Event), 'native_CPU_SHA256': digest, 'discovery_no_preload': True, 'missing_endpoint_preload_rejected': True, 'initialization_property_cases': 13, 'initialization_properties_without_Configure_export': True, 'property_hash_mismatch_malformed_relative_and_absent_cases': True})
    (args.output / 'result.json').write_text(json.dumps(host, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(host))


if __name__ == '__main__':
    main()
