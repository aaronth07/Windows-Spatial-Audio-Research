"""Cold process: APO property initialization, prepared SOFA, zero-GUID HrtfApo."""
import argparse
import ctypes as c
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import numpy as np
from scipy.signal import fftconvolve
from regfree_factory_probe import PTR, HR, U32, U64, Buffer, check, guid, method

HERE = Path(__file__).resolve().parent
BOOTSTRAP = '27e623f8-71ce-40d1-ae8c-bcac46a71243'
APO = 'fd7f2b29-24d0-4b5c-b177-592c39f9ca10'


class Key(c.Structure):
    _fields_ = [('format', c.c_ubyte * 16), ('id', U32)]


class Variant(c.Structure):
    _fields_ = [('type', c.c_uint16), ('reserved', c.c_uint16 * 3), ('value', U64 * 2)]


class Base(c.Structure):
    _fields_ = [('size', U32), ('clsid', c.c_ubyte * 16)]


class Init(c.Structure):
    _fields_ = [('base', Base), ('endpoint', PTR), ('effects', PTR), ('reserved', PTR), ('collection', PTR)]


def header(mode, banks=0, taps=0, capacity=0, payload=b'', **changes):
    values = dict(magic=b'WSSSOFA1', version=1, mode=mode, banks=banks, taps=taps,
                  capacity=capacity, reserved=0, sha=hashlib.sha256(payload).digest())
    values.update(changes)
    return struct.pack('<8s6I32s', *(values[k] for k in ('magic', 'version', 'mode', 'banks', 'taps', 'capacity', 'reserved', 'sha'))) + payload


def initialize(module_path, log):
    """Use actual SDK COM memory store and APO Initialize, with no Configure calls."""
    assert c.sizeof(Variant) == 24 and c.sizeof(Init) == 56 and Init.endpoint.offset == 24
    propsys = c.WinDLL('propsys')
    propsys.PSCreateMemoryPropertyStore.argtypes = [PTR, c.POINTER(PTR)]
    propsys.PSCreateMemoryPropertyStore.restype = HR
    store = PTR()
    check(propsys.PSCreateMemoryPropertyStore(guid('886d8eeb-8cf2-4446-8d02-cdba1dbdcf99'), c.byref(store)))
    strings = []
    def put(fmt, index, value):
        key, variant = Key(guid(fmt), index), Variant()
        if isinstance(value, bool):
            variant.type = 11
            variant.value[0] = 0xffff if value else 0
        else:
            variant.type = 31
            text = c.create_unicode_buffer(str(value)); strings.append(text)
            variant.value[0] = c.cast(text, PTR).value
        check(method(store, 6, HR, c.POINTER(Key), c.POINTER(Variant))(store, c.byref(key), c.byref(variant)))
    factory, instance = PTR(), PTR()
    bootstrap = c.WinDLL(str(HERE / 'bootstrap_apo.dll'))
    try:
        put('1da5d803-d492-4edd-8c23-e0c0ffee7f0e', 4, '{11111111-2222-4333-8444-555555555555}')
        for index, value in [(1, log), (2, module_path), (3, True),
                             (4, hashlib.sha256(module_path.read_bytes()).hexdigest())]:
            put(BOOTSTRAP, index, value)
        get = bootstrap.DllGetClassObject
        get.argtypes = [PTR, PTR, c.POINTER(PTR)]; get.restype = HR
        check(get(guid(BOOTSTRAP), guid('00000001-0000-0000-c000-000000000046'), c.byref(factory)))
        check(method(factory, 3, HR, PTR, PTR, c.POINTER(PTR))(factory, None, guid(APO), c.byref(instance)))
        init = Init(Base(c.sizeof(Init), guid(BOOTSTRAP)), store, store, None, None)
        result = method(instance, 6, HR, U32, PTR)(instance, c.sizeof(init), c.byref(init))
        return result, bootstrap, instance
    except Exception:
        if instance: method(instance, 2, U32)(instance)
        raise
    finally:
        if factory: method(factory, 2, U32)(factory)
        method(store, 2, U32)(store)


def child(folder, binaries, case):
    module = folder / 'HrtfDspCpu.dll'
    kernel = c.WinDLL('kernel32'); kernel.GetModuleHandleW.argtypes = [c.c_wchar_p]; kernel.GetModuleHandleW.restype = PTR
    assert not kernel.GetModuleHandleW('HrtfDspCpu.dll'), 'requires a cold process'
    native_first = c.WinDLL(str(binaries/'HrtfDspCpu.dll')) if case == 'native_first' else None
    ole = c.WinDLL('ole32'); ole.CoInitializeEx.argtypes = [PTR, U32]; ole.CoInitializeEx.restype = HR
    check(ole.CoInitializeEx(None, 0))
    instance, factory, engine = PTR(), PTR(), PTR(); controls = []; bootstrap = None
    try:
        hr, bootstrap, instance = initialize(module, folder / 'bootstrap.events')
        if case not in ('sofa', 'forward'):
            assert hr < 0
            if case == 'native_first':
                assert (hr & 0xffffffff) == 0x800700b7 and kernel.GetModuleHandleW('HrtfDspCpu.dll') == native_first._handle
            else:
                assert not kernel.GetModuleHandleW('HrtfDspCpu.dll')
            result = {'passed': True, 'case': case, 'initialize_hresult': hr & 0xffffffff,
                      'custom_module_rejected': True, 'audio_streams_started': 0}
        else:
            check(hr)
            server = c.WinDLL(str(module))
            for name in ('GetCreationCount', 'GetLiveDspCount', 'GetSofaCalls', 'GetSofaErrors'):
                getattr(server, name).restype = U32
            server.GetRenderAllocations.restype = U64
            assert server.GetCreationCount() == 0
            native = c.WinDLL(str(binaries / 'HrtfApo.dll'))
            native.CreateHrtfEngineFactory.argtypes = [c.POINTER(PTR)]; native.CreateHrtfEngineFactory.restype = HR
            check(native.CreateHrtfEngineFactory(c.byref(factory)))
            check(method(factory, 3, HR, PTR, U32, U32, c.POINTER(PTR))(factory, guid('00000000-0000-0000-0000-000000000000'), 2, 0, c.byref(engine)))
            assert server.GetCreationCount() == 1
            if case == 'sofa':
                raw = (folder / 'HrtfDspCpu.sofa.bin').read_bytes()
                _, _, _, count, taps, _, _, _ = struct.unpack('<8s6I32s', raw[:64])
                data = np.frombuffer(raw, dtype='<f4', offset=64)
                banks = data[:count*taps*2].reshape(count, taps, 2)
                directions = data[count*taps*2:].reshape(count, 3)
            else:
                count, taps = 0, 0
                directions = np.array([[-1, 0, 0], [1, 0, 0]], dtype=np.float32)
            indices = [0, len(directions)-1]
            for source in range(2):
                control = PTR()
                check(method(engine, 3, HR, U64, PTR, c.POINTER(PTR))(engine, source, guid('15b3cd66-e9de-4464-b6e6-2bc3cf63d455'), c.byref(control)))
                controls.append(control)
                check(method(control, 3, HR, c.POINTER(c.c_float))(control, (c.c_float*3)(*directions[indices[source]])))
            rng = np.random.RandomState(734)
            pcm = np.zeros((12, 2, 1024), dtype=np.float32)
            pcm[:4] = rng.normal(0, .001, (4, 2, 1024)).astype(np.float32)
            audio = []
            for block in pcm:
                buffers = (Buffer*2)(*(Buffer(block[s].ctypes.data_as(c.POINTER(c.c_float)), 1024) for s in range(2)))
                out = np.full(2080, 12345, dtype=np.float32); core = out[16:-16]; written = U64()
                check(method(engine, 4, HR, c.POINTER(Buffer), U64, c.POINTER(c.c_float), U64, c.POINTER(U64))(engine, buffers, 2, core.ctypes.data_as(c.POINTER(c.c_float)), 2048, c.byref(written)))
                assert written.value == 2048 and np.isfinite(core).all() and np.all(out[:16] == 12345) and np.all(out[-16:] == 12345)
                audio.append(core.reshape(-1, 2).copy())
            actual = np.concatenate(audio)
            result = {'passed': True, 'case': case, 'cold_factory_creations': server.GetCreationCount(),
                      'property_initialize_without_configure_exports': True, 'zero_GUID': True,
                      'prepared_banks': count, 'taps': taps, 'native_callbacks': 12,
                      'audio_streams_started': 0, 'system_registration': False}
            if case == 'sofa':
                expected = np.zeros_like(actual, dtype=float)
                for source in range(2):
                    samples = pcm[:, source].reshape(-1).astype(float)
                    for ear in range(2):
                        response = fftconvolve(samples, banks[indices[source], :, ear].astype(float))
                        expected[128:, ear] += response[:len(expected)-128]
                error = float(np.max(np.abs(actual-expected)))
                assert error < 2e-7 and np.max(np.abs(expected)) > 0
                assert server.GetSofaCalls() == 12 and server.GetSofaErrors() == 0 and server.GetRenderAllocations() == 0
                result.update(max_reference_error=error, full_tail=True, render_new_calls=0)
                np.savez_compressed(folder / 'comparison.npz', pcm=pcm, actual=actual, expected=expected)
            else:
                assert np.isfinite(actual).all() and np.max(np.abs(actual)) > 0 and server.GetSofaCalls() == 0
            for control in controls: method(control, 2, U32)(control)
            controls = []; method(engine, 2, U32)(engine); engine = PTR()
            assert server.GetLiveDspCount() == 0
        (folder / 'result.json').write_text(json.dumps(result, indent=2))
        print(json.dumps(result))
    finally:
        for control in controls: method(control, 2, U32)(control)
        if engine: method(engine, 2, U32)(engine)
        if factory: method(factory, 2, U32)(factory)
        if instance: method(instance, 2, U32)(instance)
        if bootstrap:
            bootstrap.CloseBootstrap.restype = HR; check(bootstrap.CloseBootstrap())
        ole.CoUninitialize()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary_directory', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--child')
    args = parser.parse_args()
    if args.child:
        child(args.output.resolve(), args.binary_directory.resolve(), args.child); return
    for name, digest in [('HrtfApo.dll', 'ffc12d088cc02441061cc2bb7f3b367fe8089662e0fd61ab98209ef6d43b6489'), ('HrtfDspCpu.dll', '4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9')]:
        if hashlib.sha256((args.binary_directory/name).read_bytes()).hexdigest() != digest:
            raise ValueError('Unreviewed binary SHA-256')
    args.output.mkdir(parents=True, exist_ok=False)
    kernel = HERE.parents[2] / 'fixtures/generated'
    raw = (kernel/'full_sofa_banks.bin').read_bytes()
    magic, count, taps, xtc, gain = struct.unpack('<4If', raw[:20])
    assert (magic, count, taps, xtc, gain) == (0x324b4e42, 710, 4096, 0, 1)
    data = raw[20:] + (kernel/'full_sofa_directions_windows.f32').read_bytes()
    valid = header(2, count, taps, 2, data)
    small = np.zeros(8195, dtype='<f4'); small[-1] = -1
    payload = small.tobytes()
    cases = {'sofa': valid, 'forward': header(0), 'native_first': header(0), 'missing': None, 'short_header': b'WSSSOFA1',
             'magic': header(0, magic=b'WRONG123'), 'version': header(0, version=2),
             'reserved': header(0, reserved=1), 'mode': header(1),
             'forward_dimensions': header(0, banks=1),
             'banks_zero': header(2, 0, 4096, 2), 'banks_large': header(2, 2049, 4096, 2),
             'taps': header(2, 1, 2048, 2, payload), 'capacity': header(2, 1, 4096, 65, payload),
             'truncated': header(2, 1, 4096, 2, payload)[:-4],
             'trailing': header(0)+b'x', 'checksum': header(2, 1, 4096, 2, payload, sha=b'\0'*32)}
    nonfinite = small.copy(); nonfinite[0] = np.nan
    cases['nonfinite'] = header(2, 1, 4096, 2, nonfinite.tobytes())
    zero_direction = small.copy(); zero_direction[-1] = 0
    cases['zero_direction'] = header(2, 1, 4096, 2, zero_direction.tobytes())
    results = []
    for name, content in cases.items():
        folder = args.output/name; folder.mkdir()
        shutil.copyfile(HERE/'regfree_factory.dll', folder/'HrtfDspCpu.dll')
        if content is not None: (folder/'HrtfDspCpu.sofa.bin').write_bytes(content)
        run = subprocess.run([sys.executable, str(Path(__file__)), str(args.binary_directory.resolve()), str(folder.resolve()), '--child', name], capture_output=True, text=True, timeout=45)
        (folder/'worker.log').write_text(run.stdout+run.stderr)
        assert run.returncode == 0, (name, run.returncode, run.stdout, run.stderr)
        results.append(json.loads((folder/'result.json').read_text()))
    result = {'passed': True, 'positive_cases': 2, 'negative_cases': len(cases)-2,
              'cases': results, 'scope': 'Fresh owned processes; actual APO memory-property Initialize and zero-GUID HrtfApo, no endpoint or system mutation.'}
    (args.output/'result.json').write_text(json.dumps(result, indent=2))
    print(json.dumps({'passed': True, 'positive': 2, 'negative': len(cases)-2}))


if __name__ == '__main__': main()
