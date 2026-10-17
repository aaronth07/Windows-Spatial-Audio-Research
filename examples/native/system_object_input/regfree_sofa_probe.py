"""Compare actual manifest-activated native SOFA DSP with full direct convolution."""
import argparse
import ctypes as c
import hashlib
import json
from pathlib import Path
import struct
import time
import netCDF4
import numpy as np
from scipy.signal import fftconvolve
from regfree_factory_probe import ActCtx, Buffer, CLASS, PTR, HR, U32, U64, check, guid, method


class Observation(c.Structure):
    _fields_ = [('call', U32), ('index', U32), ('bank', U32), ('frames', U32), ('pcm_hash', U32), ('gain', c.c_float), ('position', c.c_float * 3)]


def fnv(values):
    result = 2166136261
    for value in values.tobytes():
        result = ((result ^ value) * 16777619) & 0xffffffff
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary_directory', type=Path)
    parser.add_argument('sofa', type=Path)
    parser.add_argument('banks', type=Path)
    parser.add_argument('directions', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    binaries = args.binary_directory.resolve(strict=True)
    for name, digest in [('HrtfApo.dll', 'ffc12d088cc02441061cc2bb7f3b367fe8089662e0fd61ab98209ef6d43b6489'), ('HrtfDspCpu.dll', '4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9')]:
        if hashlib.sha256((binaries / name).read_bytes()).hexdigest() != digest:
            raise ValueError('Unreviewed binary SHA-256')
    raw = args.banks.read_bytes()
    magic, count, taps, xtc_taps, gain = struct.unpack('<4If', raw[:20])
    assert (magic, count, taps, xtc_taps, gain) == (0x324b4e42, 710, 4096, 0, 1.0)
    banks = np.frombuffer(raw, dtype='<f4', offset=20).reshape(count, taps, 2).copy()
    directions = np.fromfile(str(args.directions), dtype='<f4').reshape(count, 3)
    assert np.isfinite(banks).all() and np.allclose(np.linalg.norm(directions, axis=1), 1, atol=1e-6)
    with netCDF4.Dataset(str(args.sofa)) as sofa:
        assert sofa.SOFAConventions == 'SimpleFreeFieldHRIR'
        assert np.all(sofa['Data.SamplingRate'][:] == 48000) and np.all(sofa['Data.Delay'][:] == 0)
        assert np.array_equal(banks, np.asarray(sofa['Data.IR'][:], dtype=np.float32).transpose(0, 2, 1))
        position = np.deg2rad(np.asarray(sofa['SourcePosition'][:, :2], dtype=float))
        expected_directions = np.column_stack((-np.cos(position[:, 1]) * np.sin(position[:, 0]), np.sin(position[:, 1]), -np.cos(position[:, 1]) * np.cos(position[:, 0])))
        assert np.allclose(directions, expected_directions, atol=1e-6)
        receivers = np.asarray(sofa['ReceiverPosition'][:]).squeeze(-1)
        assert receivers[0, 1] > 0 and receivers[1, 1] < 0
    args.output.mkdir(parents=True, exist_ok=False)
    folder = Path(__file__).resolve().parent
    server = c.WinDLL(str(folder / 'regfree_factory.dll'))
    server.ConfigureCpuPath.argtypes = [c.c_wchar_p]
    server.ConfigureCpuPath.restype = HR
    server.ConfigureSofa.argtypes = [c.POINTER(c.c_float), U32, U32, c.POINTER(c.c_float), U32]
    server.ConfigureSofa.restype = HR
    server.GetSofaCalls.restype = U32
    server.GetSofaErrors.restype = U32
    server.GetRenderAllocations.restype = U64
    server.GetLiveDspCount.restype = U32
    server.GetSofaObservations.argtypes = [c.POINTER(Observation), U32]
    server.GetSofaObservations.restype = U32
    check(server.ConfigureCpuPath(str(binaries / 'HrtfDspCpu.dll')))
    fp = c.POINTER(c.c_float)
    filters_ptr = banks.ctypes.data_as(fp)
    directions_ptr = directions.ctypes.data_as(fp)
    invalid = -2147024809
    negative_parameters = 0
    for parameters in [(None, count, taps, directions_ptr, 2), (filters_ptr, 0, taps, directions_ptr, 2), (filters_ptr, 2049, taps, directions_ptr, 2), (filters_ptr, count, 2048, directions_ptr, 2), (filters_ptr, count, taps, None, 2), (filters_ptr, count, taps, directions_ptr, 0), (filters_ptr, count, taps, directions_ptr, 65)]:
        assert server.ConfigureSofa(*parameters) == invalid
        negative_parameters += 1
    small_filters = np.zeros((1, taps, 2), dtype=np.float32)
    small_directions = np.array([[1., 0., 0.]], dtype=np.float32)
    small_filters[0, 0, 0] = np.nan
    assert server.ConfigureSofa(small_filters.ctypes.data_as(fp), 1, taps, small_directions.ctypes.data_as(fp), 1) == invalid
    small_filters.fill(0)
    for bad_direction in ([0., 0., 0.], [np.nan, 0., 1.], [np.finfo(np.float32).max, 0., 1.]):
        small_directions[0] = bad_direction
        assert server.ConfigureSofa(small_filters.ctypes.data_as(fp), 1, taps, small_directions.ctypes.data_as(fp), 1) == invalid
    negative_parameters += 4
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
    factory = PTR()
    results = []
    try:
        if not kernel.ActivateActCtx(context, c.byref(cookie)):
            raise c.WinError(c.get_last_error())
        active = True
        apo = c.WinDLL(str(binaries / 'HrtfApo.dll'))
        create = apo.CreateHrtfEngineFactory
        create.argtypes = [c.POINTER(PTR)]
        create.restype = HR
        check(create(c.byref(factory)))
        for moving in (False, True):
            check(server.ConfigureSofa(banks.ctypes.data_as(c.POINTER(c.c_float)), count, taps, directions.ctypes.data_as(c.POINTER(c.c_float)), 2))
            # The owned DSP factory rejects capacities before calling native DSP creation.
            direct_factory = PTR()
            ole.CoCreateInstance.argtypes = [PTR, PTR, U32, PTR, c.POINTER(PTR)]
            ole.CoCreateInstance.restype = HR
            check(ole.CoCreateInstance(guid(CLASS), None, 1, guid('b91f3db7-302c-4ae3-83de-7735f24ef462'), c.byref(direct_factory)))
            try:
                for bad_count in (0, 3, 65):
                    rejected = PTR(1)
                    assert method(direct_factory, 3, HR, PTR, PTR, U32, c.POINTER(PTR))(direct_factory, None, None, bad_count, c.byref(rejected)) == invalid
                    assert rejected.value is None and server.GetLiveDspCount() == 0
            finally:
                method(direct_factory, 2, U32)(direct_factory)
            engine = PTR()
            controls = []
            try:
                preparation = time.perf_counter()
                check(method(factory, 3, HR, PTR, U32, U32, c.POINTER(PTR))(factory, guid(CLASS), 2, 0, c.byref(engine)))
                preparation = time.perf_counter() - preparation
                assert server.ConfigureSofa(filters_ptr, count, taps, directions_ptr, 2) == -2147418113
                for index in range(2):
                    control = PTR()
                    check(method(engine, 3, HR, U64, PTR, c.POINTER(PTR))(engine, index, guid('15b3cd66-e9de-4464-b6e6-2bc3cf63d455'), c.byref(control)))
                    controls.append(control)
                rng = np.random.RandomState(8123)
                pcm = np.zeros((20, 2, 1024), dtype=np.float32)
                pcm[:12] = (rng.normal(size=(12, 2, 1024)) * .001).astype(np.float32)
                pcm[2, 0, -1] += np.float32(.01)
                pcm[11, 1, -1] += np.float32(.006)
                chosen = [17, 71, 150, 260, 355, 450, 560, 650]
                targets = np.array([[chosen[(block // 3 + source * 3) % len(chosen)] if moving and block < 12 else chosen[source * 3] for source in range(2)] for block in range(20)])
                if moving:
                    targets[12:] = targets[11]
                audio, costs = [], []
                for block in range(20):
                    for source in range(2):
                        vector = (c.c_float * 3)(*directions[targets[block, source]])
                        check(method(controls[source], 3, HR, c.POINTER(c.c_float))(controls[source], vector))
                    buffers = (Buffer * 2)(*(Buffer(pcm[block, source].ctypes.data_as(c.POINTER(c.c_float)), 1024) for source in range(2)))
                    guarded = np.full(2048 + 32, 12345, dtype=np.float32)
                    output = guarded[16:-16]
                    written = U64()
                    before = time.perf_counter_ns()
                    check(method(engine, 4, HR, c.POINTER(Buffer), U64, c.POINTER(c.c_float), U64, c.POINTER(U64))(engine, buffers, 2, output.ctypes.data_as(c.POINTER(c.c_float)), 2048, c.byref(written)))
                    costs.append((time.perf_counter_ns() - before) / 1e6)
                    assert written.value == 2048 and np.all(guarded[:16] == 12345) and np.all(guarded[-16:] == 12345) and np.isfinite(output).all()
                    audio.append(output.reshape(1024, 2).copy())
                observations = (Observation * 40)()
                assert server.GetSofaObservations(observations, 40) == 40
                expected = np.zeros((20 * 1024 + taps + 128, 2), dtype=np.float64)
                previous = [None, None]
                for block in range(20):
                    for source in range(2):
                        observation = observations[block * 2 + source]
                        target = int(np.argmax(directions.astype(float) @ directions[targets[block, source]].astype(float)))
                        assert observation.index == source and observation.call == block + 1 and observation.bank == target and observation.frames == 1024
                        assert observation.pcm_hash == fnv(pcm[block, source]) and np.allclose(observation.position, directions[targets[block, source]], atol=2e-7) and abs(observation.gain - 1) < 1e-6
                        old = target if previous[source] is None else previous[source]
                        values = pcm[block, source].astype(np.float64) * observation.gain
                        alpha = np.arange(1024, dtype=np.float64) / 1023
                        for ear in range(2):
                            if old == target:
                                response = fftconvolve(values, banks[target, :, ear].astype(float))
                            else:
                                response = fftconvolve(values * (1 - alpha), banks[old, :, ear].astype(float)) + fftconvolve(values * alpha, banks[target, :, ear].astype(float))
                            start = block * 1024 + 128
                            expected[start:start + len(response), ear] += response
                        previous[source] = target
                actual = np.concatenate(audio)
                error = float(np.max(np.abs(actual - expected[:len(actual)])))
                assert error < 2e-7 and np.max(np.abs(actual)) > 0
                assert server.GetSofaCalls() == 20 and server.GetSofaErrors() == 0 and server.GetRenderAllocations() == 0
                np.savez_compressed(args.output / ('moving.npz' if moving else 'static.npz'), pcm=pcm, output=actual, expected=expected[:len(actual)], targets=targets)
                results.append({'moving': moving, 'max_reference_error': error, 'source_content_pose_gain_checks': 40, 'native_callbacks': 20, 'kernel_blocks': 160, 'renderer_new_calls': 0, 'engine_prepare_seconds': preparation, 'engine_wall_ms': {'median': float(np.median(costs[2:])), 'p99': float(np.percentile(costs[2:], 99)), 'max': float(max(costs[2:]))}, 'full_tail_flush_frames': 8 * 1024})
            finally:
                for control in controls:
                    method(control, 2, U32)(control)
                if engine:
                    method(engine, 2, U32)(engine)
                assert server.GetLiveDspCount() == 0
        result = {'passed': True, 'negative_configuration_parameters': negative_parameters, 'change_while_live_rejected': True, 'bank_count': count, 'taps': taps, 'ears': 2, 'source_count': 2, 'native_quantum': 1024, 'kernel_quantum_and_delay': 128, 'cases': results, 'sofa_sha256': hashlib.sha256(args.sofa.read_bytes()).hexdigest(), 'banks_sha256': hashlib.sha256(raw).hexdigest(), 'directions_sha256': hashlib.sha256(args.directions.read_bytes()).hexdigest(), 'scope': 'Actual unmodified isolated HrtfApo with manifest-activated custom COM DSP. Fixed native source indices until engine release; no system install, generic game object capture, spatial interpolation, room retention, head tracking or device latency proof.'}
        (args.output / 'result.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
        print(json.dumps(result))
    finally:
        if factory:
            method(factory, 2, U32)(factory)
        if active:
            kernel.DeactivateActCtx(0, cookie)
        kernel.ReleaseActCtx(context)
        ole.CoUninitialize()


if __name__ == '__main__':
    main()
