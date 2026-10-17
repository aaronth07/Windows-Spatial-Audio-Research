"""Test pinned AudioEng float-format admission with owned synthetic media types."""
import argparse
import ctypes as c
import hashlib
import json
from pathlib import Path
import uuid

PIN = '0177e79b1d4c578bc4c7c8804c17f02bfec129299d4b8d981786be0a490a5d6f'
PTR, U32, HR = c.c_void_p, c.c_uint32, c.c_long
CALL = c.WINFUNCTYPE


class Format(c.Structure):
    _fields_ = [('guid', c.c_ubyte * 16), ('channels', U32), ('container', U32), ('bits', U32), ('rate', c.c_float), ('mask', U32)]


class Wave(c.Structure):
    _pack_ = 1
    _fields_ = [('tag', c.c_uint16), ('channels', c.c_uint16), ('rate', U32), ('byte_rate', U32), ('align', c.c_uint16), ('bits', c.c_uint16), ('extra', c.c_uint16)]


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
    kernel.InitializeCriticalSection.argtypes = [PTR]
    kernel.DeleteCriticalSection.argtypes = [PTR]
    module = kernel.LoadLibraryExW(str(binary), None, 0x1100)
    if not module:
        raise c.WinError(c.get_last_error())
    refs = [1]
    callbacks = []
    media_table = (PTR * 7)()
    media = (PTR * 1)(c.addressof(media_table))
    fmt = Format((c.c_ubyte * 16).from_buffer_copy(uuid.UUID('00000003-0000-0010-8000-00aa00389b71').bytes_le), 16, 4, 32, 48000.0, 0xFFFF)
    wave = Wave(3, 16, 48000, 48000 * 16 * 4, 16 * 4, 32, 0)
    def install(slot, signature, action):
        callback = signature(action)
        callbacks.append(callback)
        media_table[slot] = c.cast(callback, PTR).value
    install(1, CALL(U32, PTR), lambda self: refs.__setitem__(0, refs[0] + 1) or refs[0])
    install(2, CALL(U32, PTR), lambda self: refs.__setitem__(0, refs[0] - 1) or refs[0])
    install(3, CALL(HR, PTR, c.POINTER(c.c_int)), lambda self, compressed: compressed.__setitem__(0, 0) or 0)
    install(5, CALL(PTR, PTR), lambda self: c.addressof(wave))
    install(6, CALL(HR, PTR, PTR), lambda self, target: c.memmove(target, c.byref(fmt), c.sizeof(fmt)) and 0)
    apo_table = (PTR * 13)()
    apo_table[12] = module + 0xEAA0
    apo = (c.c_ubyte * 1024)()
    PTR.from_buffer(apo).value = c.addressof(apo_table)
    kernel.InitializeCriticalSection(c.addressof(apo) + 0x28)
    try:
        normalize = CALL(c.c_bool, PTR, c.POINTER(Format))(module + 0xEAA0)
        cases = []
        for channels in (1, 2, 8, 16, 64, 4096):
            candidate = Format.from_buffer_copy(fmt)
            candidate.channels = channels
            assert not normalize(apo, c.byref(candidate)) and candidate.channels == channels
            cases.append({'channels': channels, 'modified': False})
        for value, expected in ((0, 1), (4097, 4096)):
            candidate = Format.from_buffer_copy(fmt)
            candidate.channels = value
            assert normalize(apo, c.byref(candidate)) and candidate.channels == expected
        support = CALL(HR, PTR, PTR, PTR, c.POINTER(PTR))(module + 0x70180)
        supported = PTR()
        support_hr = support(apo, None, media, c.byref(supported))
        assert support_hr == 0 and supported.value == c.addressof(media) and refs[0] == 2
        callbacks[1](media)
        # Native ValidateAndCacheConnectionInfo reads initialized state, format
        # selection, the two connection media types, and the local critical section.
        apo[0x19] = 1
        mode = uuid.UUID('4444acb0-8dc0-4c2c-a0d8-2c76db470f86').bytes_le
        c.memmove(c.addressof(apo) + 0xDC, mode, 16)
        connection = (c.c_ubyte * 40)()
        PTR.from_buffer(connection, 0x18).value = c.addressof(media)
        connections = (PTR * 1)(c.addressof(connection))
        validate = CALL(HR, PTR, U32, PTR, U32, PTR)(module + 0x136D0)
        validate_hr = validate(apo, 1, connections, 1, connections)
        assert validate_hr == 0
        result = {'binary_sha256': PIN, 'normalizer_cases': cases, 'invalid_channel_counts_clamped': True, 'IsOutputFormatSupported_16_float_hr': support_hr, 'supported_media_identity_exact': True, 'ValidateAndCacheConnectionInfo_16_float_hr': validate_hr, 'output_format': {'channels': 16, 'rate': 48000, 'bits': 32, 'subtype': 'IEEE float'}, 'scope': 'Actual local format methods with owned synthetic media type and APO state; not real endpoint negotiation, provider selection, codec operation, or Windows pipeline execution.'}
        (args.output / 'result.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
        print(json.dumps(result))
    finally:
        kernel.DeleteCriticalSection(c.addressof(apo) + 0x28)
        if not kernel.FreeLibrary(module):
            raise c.WinError(c.get_last_error())


if __name__ == '__main__':
    main()
