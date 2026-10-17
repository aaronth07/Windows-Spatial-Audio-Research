"""Verify actual HrtfApo CoCreateInstance through a process-local COM manifest."""
import argparse
import ctypes as c
import hashlib
import json
import math
from pathlib import Path
import uuid

PTR, HR, U32, U64 = c.c_void_p, c.c_long, c.c_uint32, c.c_uint64
CALL = c.WINFUNCTYPE
CLASS = '1eb4a2d1-0aec-4dfa-a02c-c944cb6f74e2'


class ActCtx(c.Structure):
    _fields_ = [('size', U32), ('flags', U32), ('source', c.c_wchar_p), ('architecture', c.c_uint16), ('language', c.c_uint16), ('directory', c.c_wchar_p), ('resource', c.c_wchar_p), ('application', c.c_wchar_p), ('module', PTR)]


class Buffer(c.Structure):
    _fields_ = [('samples', c.POINTER(c.c_float)), ('frames', U64)]


def guid(value):
    return (c.c_ubyte * 16).from_buffer_copy(uuid.UUID(value).bytes_le)


def method(obj, slot, result, *arguments):
    table = c.cast(obj, c.POINTER(PTR))[0]
    return CALL(result, PTR, *arguments)(c.cast(table, c.POINTER(PTR))[slot])


def check(hr):
    if hr < 0:
        raise RuntimeError('HRESULT 0x%08x' % (hr & 0xffffffff))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary_directory', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    binaries = args.binary_directory.resolve(strict=True)
    digests = {'HrtfApo.dll': 'ffc12d088cc02441061cc2bb7f3b367fe8089662e0fd61ab98209ef6d43b6489', 'HrtfDspCpu.dll': '4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9'}
    for name, expected in digests.items():
        if hashlib.sha256((binaries / name).read_bytes()).hexdigest() != expected:
            raise ValueError('Unreviewed binary SHA-256: ' + name)
    if c.sizeof(PTR) != 8:
        raise RuntimeError('Requires x64 Python')
    args.output.mkdir(parents=True, exist_ok=False)
    folder = Path(__file__).resolve().parent
    server = c.WinDLL(str(folder / 'regfree_factory.dll'))
    server.ConfigureCpuPath.argtypes = [c.c_wchar_p]
    server.ConfigureCpuPath.restype = HR
    server.GetCreationCount.restype = U32
    server.ConfigureDiagnostic.argtypes = [c.c_int]
    server.ConfigureDiagnostic.restype = HR
    server.GetDiagnosticCalls.restype = U32
    server.GetDiagnosticErrors.restype = U32
    server.GetLiveDspCount.restype = U32
    check(server.ConfigureCpuPath(str(binaries / 'HrtfDspCpu.dll')))
    assert server.ConfigureCpuPath(None) == -2147024809
    ole = c.WinDLL('ole32')
    ole.CoInitializeEx.argtypes = [PTR, U32]
    ole.CoInitializeEx.restype = HR
    ole.CoCreateInstance.argtypes = [PTR, PTR, U32, PTR, c.POINTER(PTR)]
    ole.CoCreateInstance.restype = HR
    check(ole.CoInitializeEx(None, 0))
    kernel = c.WinDLL('kernel32', use_last_error=True)
    kernel.CreateActCtxW.argtypes = [c.POINTER(ActCtx)]
    kernel.CreateActCtxW.restype = PTR
    kernel.ActivateActCtx.argtypes = [PTR, c.POINTER(c.c_size_t)]
    kernel.DeactivateActCtx.argtypes = [U32, c.c_size_t]
    kernel.ReleaseActCtx.argtypes = [PTR]
    factory_iid = guid('b91f3db7-302c-4ae3-83de-7735f24ef462')
    cls = guid(CLASS)
    direct = PTR()
    absent_hr = ole.CoCreateInstance(cls, None, 1, factory_iid, c.byref(direct))
    assert absent_hr < 0 and not direct
    manifest = str(folder / 'regfree_factory.manifest')
    info = ActCtx(size=c.sizeof(ActCtx), source=manifest)
    context = kernel.CreateActCtxW(c.byref(info))
    if context == PTR(-1).value:
        raise c.WinError(c.get_last_error())
    cookie = c.c_size_t()
    activated = False
    engine_factory = PTR()
    try:
        if not kernel.ActivateActCtx(context, c.byref(cookie)):
            raise c.WinError(c.get_last_error())
        activated = True
        check(ole.CoCreateInstance(cls, None, 1, factory_iid, c.byref(direct)))
        method(direct, 2, U32)(direct)
        direct = PTR()
        apo = c.WinDLL(str(binaries / 'HrtfApo.dll'))
        create = apo.CreateHrtfEngineFactory
        create.argtypes = [c.POINTER(PTR)]
        create.restype = HR
        check(create(c.byref(engine_factory)))
        controls_iid = guid('15b3cd66-e9de-4464-b6e6-2bc3cf63d455')
        rendered = []
        for mode, selection in enumerate((guid('00000000-0000-0000-0000-000000000000'), cls, cls)):
            check(server.ConfigureDiagnostic(mode == 2))
            engine = PTR()
            controls = []
            try:
                check(method(engine_factory, 3, HR, PTR, U32, U32, c.POINTER(PTR))(engine_factory, selection, 2, 0, c.byref(engine)))
                if mode:
                    assert server.ConfigureDiagnostic(mode != 2) == -2147418113
                for index, position in enumerate(((-1, 0, 0), (1, 0, 0))):
                    control = PTR()
                    check(method(engine, 3, HR, U64, PTR, c.POINTER(PTR))(engine, index, controls_iid, c.byref(control)))
                    controls.append(control)
                    xyz = (c.c_float * 3)(*position)
                    check(method(control, 3, HR, c.POINTER(c.c_float))(control, xyz))
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
                    assert written.value == 2048 and all(map(math.isfinite, output))
                    values.extend(output)
                assert max(map(abs, values)) > 0
                rendered.append(values)
            finally:
                for control in controls:
                    method(control, 2, U32)(control)
                if engine:
                    method(engine, 2, U32)(engine)
            assert server.GetLiveDspCount() == 0
        expected = [0.0] * len(rendered[2])
        expected[3072 * 2] = c.c_float(0.01).value
        expected[4096 * 2 + 1] = c.c_float(0.005).value
        assert rendered[0] == rendered[1] and server.GetCreationCount() == 3
        assert rendered[2] == expected and server.GetDiagnosticCalls() == 12 and server.GetDiagnosticErrors() == 0
        result = {'passed': True, 'manifest_clsid': CLASS, 'without_activation_context_hr': hex(absent_hr & 0xffffffff), 'real_CoCreateInstance_factory_creations': server.GetCreationCount(), 'native_engine_sources': 2, 'frames_per_block': 1024, 'blocks_per_engine': 12, 'baseline_and_forwarding_DSP_all_samples_exact': True, 'custom_DSP_expected_PCM_all_samples_exact': True, 'custom_DSP_callbacks': server.GetDiagnosticCalls(), 'custom_DSP_source_pose_frame_errors': server.GetDiagnosticErrors(), 'live_custom_DSPs_after_release': server.GetLiveDspCount(), 'change_while_live_rejected': True, 'null_path_rejected': True, 'compared_stereo_samples_per_case': len(rendered[0]), 'binary_sha256': digests, 'scope': 'Real unmodified HrtfApo nonzero-factory CoCreateInstance with process-local manifest, owned COM factory and separate custom DSP object; no original IAT/vtable modification, registry registration, system selection, HRTF convolution, or room-retention proof.'}
        (args.output / 'result.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
        print(json.dumps(result))
    finally:
        if engine_factory:
            method(engine_factory, 2, U32)(engine_factory)
        if direct:
            method(direct, 2, U32)(direct)
        if activated:
            kernel.DeactivateActCtx(0, cookie)
        kernel.ReleaseActCtx(context)
        ole.CoUninitialize()


if __name__ == '__main__':
    main()
