"""Exercise pinned AudioEng provider-wrapper calls with an owned synthetic provider."""
import argparse
import ctypes as c
import hashlib
import json
import struct
from pathlib import Path
import uuid

PIN = '0177e79b1d4c578bc4c7c8804c17f02bfec129299d4b8d981786be0a490a5d6f'
PTR, U32, HR = c.c_void_p, c.c_uint32, c.c_long
CALL = c.WINFUNCTYPE
PRIMARY = 'deff1192-f581-4d77-9c1b-3e596b0ca989'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    binary = args.binary.resolve(strict=True)
    if hashlib.sha256(binary.read_bytes()).hexdigest() != PIN:
        raise ValueError('Unreviewed binary SHA-256')
    if c.sizeof(PTR) != 8:
        raise RuntimeError('Requires x64 Python')
    args.output.mkdir(parents=True, exist_ok=False)
    kernel = c.WinDLL('kernel32', use_last_error=True)
    kernel.LoadLibraryExW.argtypes = [c.c_wchar_p, PTR, U32]
    kernel.LoadLibraryExW.restype = PTR
    kernel.FreeLibrary.argtypes = [PTR]
    kernel.FreeLibrary.restype = c.c_int
    module = kernel.LoadLibraryExW(str(binary), None, 0x1100)
    if not module:
        raise c.WinError(c.get_last_error())
    events, errors = [], []
    refs = [1]
    fail_primary = [False]
    callbacks = []
    table = (PTR * 11)()
    provider = (PTR * 1)(c.addressof(table))
    provider_address = c.addressof(provider)
    wrapper = (PTR * 4)()
    context = (c.c_ubyte * 32)()

    def callback(slot, signature, action):
        def protected(*values):
            try:
                return action(*values)
            except Exception as error:
                errors.append(repr(error))
                return -2147467259
        function = signature(protected)
        callbacks.append(function)
        table[slot] = c.cast(function, PTR).value

    def query(self, iid, out):
        name = str(uuid.UUID(bytes_le=c.string_at(iid, 16)))
        out[0] = None
        events.append({'call': 'QueryInterface', 'iid': name})
        if name == PRIMARY and not fail_primary[0]:
            refs[0] += 1
            out[0] = provider_address
            return 0
        return -2147467262

    callback(0, CALL(HR, PTR, PTR, c.POINTER(PTR)), query)
    callback(1, CALL(U32, PTR), lambda self: refs.__setitem__(0, refs[0] + 1) or refs[0])
    callback(2, CALL(U32, PTR), lambda self: refs.__setitem__(0, refs[0] - 1) or refs[0])

    def initialize(self, frames, rate, static_mask, channels, wave_mask, mapper, diagnostics, runtime):
        assert self == provider_address
        events.append({'call': 'Initialize', 'max_frames': frames, 'input_rate': rate, 'static_mask': static_mask, 'output_channels': channels, 'output_wave_mask': wave_mask, 'contexts': [mapper, diagnostics, runtime]})
        return 0
    callback(3, CALL(HR, PTR, U32, U32, U32, U32, U32, PTR, PTR, PTR), initialize)

    def add_object(self, stream, descriptor, pcm, frames, gain):
        raw = c.string_at(descriptor, 180)
        source = list(c.cast(pcm, c.POINTER(c.c_float))[:frames])
        events.append({'call': 'SubmitObject', 'stream_hex': c.string_at(stream, 16).hex(), 'descriptor_hex': raw.hex(), 'frames': frames, 'master_gain': gain, 'pcm': source})
        return 0
    callback(4, CALL(HR, PTR, PTR, PTR, PTR, U32, c.c_float), add_object)
    callback(5, CALL(HR, PTR, PTR, PTR, PTR, PTR, U32, c.c_float), lambda self, stream, descriptor, metadata, pcm, frames, gain: events.append({'call': 'SubmitMetadata', 'frames': frames, 'master_gain': gain, 'metadata_pointer_preserved': metadata == c.addressof(context)}) or 0)
    callback(6, CALL(HR, PTR, PTR, U32, U32, c.c_float, U32), lambda self, pcm, frames, channels, gain, flags: events.append({'call': 'SubmitBed', 'frames': frames, 'channels': channels, 'master_gain': gain, 'flags': flags}) or 0)

    def output(self, capacity, pcm, written):
        assert self == provider_address and capacity == 64 * 16 * 4
        values = c.cast(pcm, c.POINTER(c.c_float))
        for i in range(64 * 16):
            values[i] = (i % 16 + 1) / 32.0
        written[0] = capacity
        events.append({'call': 'Output', 'capacity_bytes': capacity, 'written_bytes': written[0]})
        return 0
    callback(7, CALL(HR, PTR, U32, PTR, c.POINTER(U32)), output)
    try:
        init = CALL(HR, PTR, PTR, PTR, PTR, PTR, U32, U32, U32, PTR, U32, U32, PTR, PTR, PTR)(module + 0x71370)
        init_hr = init(wrapper, provider, None, None, context, 480, 48000, 6, None, 16, 0xFFFF, None, None, None)
        assert init_hr == 0 and wrapper[1] == provider_address and refs[0] == 2
        descriptor = (c.c_ubyte * 180).from_buffer_copy(struct.pack('<4I4f', 7, 64, 1, 0, 1.0, 0.0, -1.0, 0.75) + bytes(148))
        stream = (c.c_ubyte * 16)(*range(16))
        pcm = (c.c_float * 64)(*[(i - 32) / 128 for i in range(64)])
        submit = CALL(HR, PTR, PTR, PTR, PTR, U32, c.c_float)(module + 0x2FBB0)
        assert submit(wrapper, stream, descriptor, pcm, 64, 0.5) == 0
        record = events[-1]
        assert record['descriptor_hex'] == bytes(descriptor).hex() and record['pcm'] == list(pcm)
        assert record['stream_hex'] == bytes(stream).hex() and record['master_gain'] == 0.5
        metadata = CALL(HR, PTR, PTR, PTR, PTR, PTR, U32, c.c_float)(module + 0x45B00)
        assert metadata(wrapper, stream, descriptor, context, pcm, 64, 0.25) == 0
        assert events[-1]['metadata_pointer_preserved']
        bed = CALL(HR, PTR, PTR, U32, U32, c.c_float, U32)(module + 0x2FB70)
        assert bed(wrapper, pcm, 4, 16, 0.75, 1) == 0
        guarded = (c.c_float * (64 * 16 + 16))(*([12345.0] * (64 * 16 + 16)))
        destination = c.cast(c.byref(guarded, 8 * 4), PTR)
        written = U32()
        render = CALL(HR, PTR, U32, U32, PTR, c.POINTER(U32))(module + 0x31D20)
        assert render(wrapper, 64, 64 * 16 * 4, destination, c.byref(written)) == 0
        assert written.value == 64 * 16 * 4
        assert list(guarded[:8]) == [12345.0] * 8 and list(guarded[-8:]) == [12345.0] * 8
        assert list(guarded[8:-8]) == [(i % 16 + 1) / 32.0 for i in range(64 * 16)]
        callbacks[2](provider_address)
        wrapper[1] = None
        fail_primary[0] = True
        reject_hr = init(wrapper, provider, None, None, context, 480, 48000, 6, None, 16, 0xFFFF, None, None, None)
        assert reject_hr == -2147467262 and not wrapper[1] and refs[0] == 1
        assert not errors
        result = {'binary_sha256': PIN, 'provider_iid': PRIMARY, 'init_hr': init_hr, 'unsupported_iid_hr': hex(reject_hr & 0xffffffff), 'all_forwarded_bytes_and_parameters_exact': True, 'output_channels': 16, 'output_frames': 64, 'output_written_bytes': written.value, 'output_canaries': True, 'events': events, 'scope': 'Actual pinned wrapper entry points with owned synthetic provider; not full MainPluginRenderer, endpoint activation, system provider selection, native panning, callback deadline, or physical output proof.'}
        (args.output / 'result.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
        print(json.dumps({key: value for key, value in result.items() if key != 'events'}))
    finally:
        if not kernel.FreeLibrary(module):
            raise c.WinError(c.get_last_error())


if __name__ == '__main__':
    main()
