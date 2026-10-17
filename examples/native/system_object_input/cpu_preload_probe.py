"""Test own-process DLL preload against HrtfApo's zero-GUID CPU factory path."""
import argparse
import ctypes as c
import hashlib
import json
from pathlib import Path
import shutil
from regfree_factory_probe import Buffer, PTR, HR, U32, U64, check, guid, method


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary_directory', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    binaries = args.binary_directory.resolve(strict=True)
    for name, digest in [('HrtfApo.dll', 'ffc12d088cc02441061cc2bb7f3b367fe8089662e0fd61ab98209ef6d43b6489'), ('HrtfDspCpu.dll', '4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9')]:
        if hashlib.sha256((binaries / name).read_bytes()).hexdigest() != digest:
            raise ValueError('Unreviewed binary SHA-256')
    args.output.mkdir(parents=True, exist_ok=False)
    server_path = args.output.resolve() / 'HrtfDspCpu.dll'
    shutil.copyfile(Path(__file__).resolve().parent / 'regfree_factory.dll', server_path)
    server = c.WinDLL(str(server_path))
    server.ConfigureCpuPath.argtypes = [c.c_wchar_p]
    server.ConfigureCpuPath.restype = HR
    server.ConfigureDiagnostic.argtypes = [c.c_int]
    server.ConfigureDiagnostic.restype = HR
    server.GetCreationCount.restype = U32
    server.GetDiagnosticCalls.restype = U32
    server.GetDiagnosticErrors.restype = U32
    server.GetLiveDspCount.restype = U32
    check(server.ConfigureCpuPath(str(binaries / 'HrtfDspCpu.dll')))
    check(server.ConfigureDiagnostic(True))
    ole = c.WinDLL('ole32')
    ole.CoInitializeEx.argtypes = [PTR, U32]
    ole.CoInitializeEx.restype = HR
    check(ole.CoInitializeEx(None, 0))
    factory, engine = PTR(), PTR()
    controls = []
    try:
        apo = c.WinDLL(str(binaries / 'HrtfApo.dll'))
        create = apo.CreateHrtfEngineFactory
        create.argtypes = [c.POINTER(PTR)]
        create.restype = HR
        check(create(c.byref(factory)))
        check(method(factory, 3, HR, PTR, U32, U32, c.POINTER(PTR))(factory, guid('00000000-0000-0000-0000-000000000000'), 2, 0, c.byref(engine)))
        for index, x in enumerate((-1, 1)):
            control = PTR()
            check(method(engine, 3, HR, U64, PTR, c.POINTER(PTR))(engine, index, guid('15b3cd66-e9de-4464-b6e6-2bc3cf63d455'), c.byref(control)))
            controls.append(control)
            check(method(control, 3, HR, c.POINTER(c.c_float))(control, (c.c_float * 3)(x, 0, 0)))
        values = []
        for block in range(12):
            left = (c.c_float * 1024)()
            right = (c.c_float * 1024)()
            if block == 3:
                left[0] = 0.01
            if block == 4:
                right[0] = 0.005
            buffers = (Buffer * 2)(Buffer(left, 1024), Buffer(right, 1024))
            output = (c.c_float * 2048)()
            written = U64()
            check(method(engine, 4, HR, c.POINTER(Buffer), U64, c.POINTER(c.c_float), U64, c.POINTER(U64))(engine, buffers, 2, output, 2048, c.byref(written)))
            assert written.value == 2048
            values.extend(output)
        intercepted = server.GetCreationCount() == 1
        expected = [0.0] * len(values)
        expected[3072 * 2] = c.c_float(0.01).value
        expected[4096 * 2 + 1] = c.c_float(0.005).value
        if intercepted:
            assert values == expected and server.GetDiagnosticCalls() == 12 and not server.GetDiagnosticErrors()
        result = {'zero_GUID_selected_preloaded_owned_factory': intercepted, 'custom_factory_creations': server.GetCreationCount(), 'custom_DSP_callbacks': server.GetDiagnosticCalls(), 'expected_PCM_exact': values == expected, 'compared_stereo_samples': len(values), 'scope': 'Fresh owned process only; preloaded own copied DLL with matching basename, no system DLL changes, registry, manifest, IAT/vtable modification, audio endpoint, or audiodg insertion.'}
        (args.output / 'result.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
        print(json.dumps(result))
    finally:
        for control in controls:
            method(control, 2, U32)(control)
        if engine:
            method(engine, 2, U32)(engine)
        if factory:
            method(factory, 2, U32)(factory)
        ole.CoUninitialize()


if __name__ == '__main__':
    main()
