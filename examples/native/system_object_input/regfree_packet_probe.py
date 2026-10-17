"""Test packet rejection/splitting with the actual manifest-activated custom DSP."""
import argparse
import ctypes as c
import hashlib
import json
import math
from pathlib import Path
from regfree_factory_probe import ActCtx, Buffer, CLASS, PTR, HR, U32, U64, check, guid, method


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
    folder = Path(__file__).resolve().parent
    server = c.WinDLL(str(folder / 'regfree_factory.dll'))
    server.ConfigureCpuPath.argtypes = [c.c_wchar_p]
    server.ConfigureCpuPath.restype = HR
    server.ConfigureDiagnostic.argtypes = [c.c_int]
    server.ConfigureDiagnostic.restype = HR
    server.GetDiagnosticCalls.restype = U32
    server.GetDiagnosticErrors.restype = U32
    server.GetLiveDspCount.restype = U32
    check(server.ConfigureCpuPath(str(binaries / 'HrtfDspCpu.dll')))
    check(server.ConfigureDiagnostic(True))
    ole = c.WinDLL('ole32')
    ole.CoInitializeEx.argtypes = [PTR, U32]
    ole.CoInitializeEx.restype = HR
    check(ole.CoInitializeEx(None, 0))
    kernel = c.WinDLL('kernel32', use_last_error=True)
    kernel.CreateActCtxW.argtypes = [c.POINTER(ActCtx)]
    kernel.CreateActCtxW.restype = PTR
    kernel.ActivateActCtx.argtypes = [PTR, c.POINTER(c.c_size_t)]
    kernel.DeactivateActCtx.argtypes = [U32, c.c_size_t]
    kernel.ReleaseActCtx.argtypes = [PTR]
    info = ActCtx(size=c.sizeof(ActCtx), source=str(folder / 'regfree_factory.manifest'))
    context = kernel.CreateActCtxW(c.byref(info))
    if context == PTR(-1).value:
        raise c.WinError(c.get_last_error())
    cookie = c.c_size_t()
    active = False
    factory, engine = PTR(), PTR()
    controls = []
    try:
        if not kernel.ActivateActCtx(context, c.byref(cookie)):
            raise c.WinError(c.get_last_error())
        active = True
        apo = c.WinDLL(str(binaries / 'HrtfApo.dll'))
        apo.GetHrtfEngineMinFrameCount.restype = U32
        exported_minimum = apo.GetHrtfEngineMinFrameCount()
        create = apo.CreateHrtfEngineFactory
        create.argtypes = [c.POINTER(PTR)]
        create.restype = HR
        check(create(c.byref(factory)))
        check(method(factory, 3, HR, PTR, U32, U32, c.POINTER(PTR))(factory, guid(CLASS), 2, 0, c.byref(engine)))
        quantum = U32.from_address(engine.value + 0x90).value
        for index, x in enumerate((-1, 1)):
            control = PTR()
            check(method(engine, 3, HR, U64, PTR, c.POINTER(PTR))(engine, index, guid('15b3cd66-e9de-4464-b6e6-2bc3cf63d455'), c.byref(control)))
            controls.append(control)
            check(method(control, 3, HR, c.POINTER(c.c_float))(control, (c.c_float * 3)(x, 0, 0)))
        rows = []
        for frames in (128, 256, 480, 512, 1024, 2048):
            first = (c.c_float * frames)(*[(i % 19) / 1024 for i in range(frames)])
            second = (c.c_float * frames)(*[-(i % 23) / 2048 for i in range(frames)])
            buffers = (Buffer * 2)(Buffer(first, frames), Buffer(second, frames))
            guarded = (c.c_float * (8192 + 32))(*([12345.0] * (8192 + 32)))
            output = c.cast(c.byref(guarded, 16 * 4), c.POINTER(c.c_float))
            written = U64()
            before = server.GetDiagnosticCalls()
            hr = method(engine, 4, HR, c.POINTER(Buffer), U64, c.POINTER(c.c_float), U64, c.POINTER(U64))(engine, buffers, 2, output, 8192, c.byref(written))
            callbacks = server.GetDiagnosticCalls() - before
            if frames < quantum:
                assert hr == -2147024809 and written.value == 0 and callbacks == 0
                assert list(guarded) == [12345.0] * len(guarded)
            else:
                check(hr)
                assert written.value == 2 * frames and callbacks == frames // quantum
                expected = [value for pair in zip(first, second) for value in pair]
                assert list(output[:2 * frames]) == expected and all(map(math.isfinite, output[:2 * frames]))
                assert list(guarded[:16]) == [12345.0] * 16 and list(guarded[16 + 2 * frames:]) == [12345.0] * (len(guarded) - 16 - 2 * frames)
            rows.append({'packet_frames': frames, 'hr': hex(hr & 0xffffffff), 'callback_count': callbacks, 'written_stereo_samples': written.value, 'exact_PCM_and_bounds': True})
        assert server.GetDiagnosticErrors() == 0
        result = {'engine_quantum_field_0x90': quantum, 'exported_GetHrtfEngineMinFrameCount': exported_minimum, 'custom_DSP_accepts_variable_frames': True, 'rows': rows, 'scope': 'Owned native HrtfApo frontend with manifest-activated custom DSP; no private engine state modified, system selection, endpoint period, device queue or latency measurement.'}
        (args.output / 'result.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
        print(json.dumps(result))
    finally:
        for control in controls:
            method(control, 2, U32)(control)
        if engine:
            method(engine, 2, U32)(engine)
        if factory:
            method(factory, 2, U32)(factory)
        if active:
            kernel.DeactivateActCtx(0, cookie)
        kernel.ReleaseActCtx(context)
        ole.CoUninitialize()


if __name__ == '__main__':
    main()
