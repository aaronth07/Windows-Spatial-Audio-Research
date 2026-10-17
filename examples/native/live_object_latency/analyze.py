"""Offline PN correlation and timestamp analysis. Requires NumPy and SciPy."""
import argparse
import hashlib
import json
from pathlib import Path
import sys
import numpy as np
from scipy.signal import correlate, fftconvolve

RATE, NOISE_FRAMES, SEEDS = 48000, 9600, (0x243f6a88, 0xb7e15162)


def require(condition, text):
    if not condition:
        raise ValueError(text)


def pn(seed, count=NOISE_FRAMES):
    values = np.random.RandomState(seed).randint(0, 2 ** 32, count, dtype=np.uint32)
    return (((values >> 9).astype(np.float32) / np.float32(8388608)) - np.float32(.5)) * np.float32(.02)


def identify(sources, stereo):
    require(stereo.ndim == 2 and stereo.shape[1] == 2, 'stereo capture required')
    require(all(np.isfinite(x).all() and len(x) > 100 for x in sources) and np.isfinite(stereo).all(), 'nonfinite or short input')
    require(np.max(np.abs(stereo)) > 1e-12, 'silent or negligible loopback capture')
    cross = correlate(sources[0], sources[1], mode='full', method='fft')
    source_cross = float(np.max(np.abs(cross)) / np.sqrt(np.dot(sources[0], sources[0]) * np.dot(sources[1], sources[1])))
    require(source_cross < .1, 'source PN sequences are too correlated')
    peaks, kernels = [], []
    for stream, source in enumerate(sources):
        energy = float(np.dot(source, source))
        per_ear, per_kernel = [], []
        for ear in range(2):
            raw = correlate(stereo[:, ear], source, mode='full', method='fft')
            # Only lags containing the entire submitted PN are considered.
            valid = raw[len(source) - 1:len(stereo)]
            require(len(valid) > 4096, 'capture too short for lag and background checks')
            lag = int(np.argmax(np.abs(valid)))
            peak = float(valid[lag])
            segment = stereo[lag:lag + len(source), ear]
            rho = abs(peak) / np.sqrt(energy * float(np.dot(segment, segment)))
            background = valid.copy()
            outside = np.ones(len(valid), dtype=bool)
            outside[max(0, lag - 4096):min(len(valid), lag + 4097)] = False
            background = background[outside]
            require(len(background) > 100, 'insufficient off-peak correlation region')
            rms = float(np.sqrt(np.mean(background ** 2)))
            snr = float(20 * np.log10(abs(peak) / max(rms, 1e-300)))
            require(np.isfinite(rho) and rho >= .08 and snr >= 10, 'PN peak not identifiable above background')
            # Matched-filter IR estimates retain the correlation noise floor.
            # They are not exact deconvolutions of an arbitrary long renderer.
            per_kernel.append((valid, energy))
            per_ear.append({'ear': ear, 'peak_lag_frames': lag, 'peak_correlation': peak,
                            'normalized_peak_correlation': float(rho), 'peak_to_offpeak_rms_db': snr,
                            'offpeak_correlation_rms': rms})
        # One shift per stereo pair preserves the measured interaural delay.
        reference_lag = min(value['peak_lag_frames'] for value in per_ear)
        aligned = []
        for valid, energy in per_kernel:
            indices = reference_lag + np.arange(-64, 4096)
            kernel = np.zeros(len(indices))
            usable = (indices >= 0) & (indices < len(valid))
            kernel[usable] = valid[indices[usable]] / energy
            aligned.append(kernel)
        peaks.append(per_ear)
        kernels.append(np.array(aligned).T)
    difference = [peaks[1][ear]['peak_lag_frames'] - peaks[0][ear]['peak_lag_frames'] for ear in range(2)]
    require(abs(difference[0] - difference[1]) <= 2, 'ears disagree on stream lag difference')
    cosine = []
    for ear in range(2):
        a, b = kernels[0][:576, ear], kernels[1][:576, ear]
        cosine.append(float(np.dot(a, b) / np.sqrt(np.dot(a, a) * np.dot(b, b))))
    return {'source_max_normalized_cross_correlation': source_cross, 'streams': peaks,
            'matched_ir_reference_capture_lag_per_stream': [min(p['peak_lag_frames'] for p in ears) for ears in peaks],
            'stream1_minus_stream0_peak_lag_frames_per_ear': difference,
            'common_peak_aligned_ir_cosine_first_512_after_peak': cosine,
            'common_ir_shape_supported': all(value >= .8 for value in cosine)}, kernels


def self_test():
    a, b = pn(SEEDS[0]).astype(float), pn(SEEDS[1]).astype(float)
    stereo = np.zeros((30000, 2))
    delays = [2400, 2437]
    filters = [np.array([.1, .8, -.2, .05]), np.array([0, .1, .7, -.1])]
    for source, delay in zip((a, b), delays):
        for ear, kernel in enumerate(filters):
            rendered = fftconvolve(source, kernel) * .001
            stereo[delay:delay + len(rendered), ear] += rendered
    result, kernels = identify((a, b), stereo)
    require(result['streams'][0][0]['peak_lag_frames'] == 2401, 'known stream0 peak')
    require(result['streams'][1][0]['peak_lag_frames'] == 2438, 'known stream1 peak')
    require(result['stream1_minus_stream0_peak_lag_frames_per_ear'] == [37, 37], 'known stream difference')
    require(result['common_ir_shape_supported'], 'known common filter shape')
    require(all(int(np.argmax(np.abs(kernel[:, 0]))) == 64 and int(np.argmax(np.abs(kernel[:, 1]))) == 65 for kernel in kernels), 'stereo IR relative delay preserved')
    cases = 5
    for name, sources, output in [
        ('silent', (a, b), stereo * 0),
        ('correlated_sources', (a, a), stereo),
        ('unrelated_noise', (a, b), np.random.RandomState(17).normal(0, .01, stereo.shape)),
        ('nonfinite', (a, b), np.full_like(stereo, np.nan)),
    ]:
        failed = False
        try: identify(sources, output)
        except ValueError: failed = True
        require(failed, 'negative case failed: ' + name)
        cases += 1
    return {'passed': True, 'offline_correlation_cases': cases, 'known_lag_difference_frames': 37, 'audio_streams_started': 0}


def analyze(folder, output):
    native = json.loads((folder / 'result.json').read_text())
    require(native['native_passed'] and native['finite'], 'native run incomplete')
    sources = [np.fromfile(str(folder / ('source%d.f32' % i)), dtype='<f4').astype(float) for i in range(2)]
    for i, source in enumerate(sources):
        require(np.array_equal(source, pn(SEEDS[i]).astype(float)), 'independent MT19937 source reconstruction failed')
        require(len(source) == NOISE_FRAMES and np.max(np.abs(source)) <= .01, 'source extent/amplitude')
    stereo = np.fromfile(str(folder / 'loopback.f32'), dtype='<f4')
    require(stereo.size == native['captured_frames'] * 2, 'loopback file truncated or incorrect extent')
    stereo = stereo.reshape(-1, 2).astype(float)
    packets = json.loads((folder / 'packets.json').read_text())['records']
    require(len(packets) == native['packets'] and len(packets) > 0, 'packet record extent')
    next_frame, next_device, last_qpc = 0, None, None
    for index, packet in enumerate(packets):
        require(packet['get_hresult'] == packet['release_hresult'] == 0, 'failed capture packet HRESULT')
        require(packet['frame_offset'] == next_frame and packet['frames'] > 0, 'packet storage extent')
        require(not (packet['flags'] & 4), 'capture timestamp error')
        require(index == 0 or not (packet['flags'] & 1), 'capture discontinuity after first packet')
        require(next_device is None or packet['device_position'] == next_device, 'capture device-position gap')
        require(last_qpc is None or packet['qpc_100ns'] > last_qpc, 'capture QPC regression')
        require(packet['host_before'] <= packet['host_after'], 'capture call bracket order')
        last_qpc = packet['qpc_100ns']
        next_frame += packet['frames']; next_device = packet['device_position'] + packet['frames']
    require(next_frame == len(stereo), 'packet sample coverage')
    result, kernels = identify(sources, stereo)
    clocks = json.loads((folder / 'clocks.json').read_text())['records']
    updates = json.loads((folder / 'updates.json').read_text())['records']
    qpc_frequency = native['host_qpc_frequency']
    origins, first_end_times, post_start_times = [], [], []
    for stream in range(2):
        service = native['streams'][stream]
        require(service['clock_service_hresult'] == service['frequency_hresult'] == 0 and service['clock_frequency'] > 0, 'stream clock unavailable')
        rows = [r for r in clocks if r['stream'] == stream]
        require(all(r['position_hresult'] == 0 for r in rows), 'GetPosition failed')
        require(all(b['position'] >= a['position'] and b['qpc_100ns'] >= a['qpc_100ns'] for a, b in zip(rows, rows[1:])), 'clock regression')
        frequency = service['clock_frequency']
        values = [r['qpc_100ns'] / 1e7 - r['position'] / frequency for r in rows if r['position'] > 0]
        require(len(values) > 10, 'insufficient progressing clock samples')
        origins.append(float(np.median(values)))
        post_start_times.append(next(r['qpc_100ns'] / 1e7 for r in rows if r['phase'] == 1))
        submitted = [r for r in updates if r['stream'] == stream and r['valid_frames'] > 0]
        require(sum(r['valid_frames'] for r in submitted) == native['source_frames'], 'submitted frame sum')
        require(submitted[0]['source_frame_offset'] == 0, 'first submitted source offset')
        require(all(r['end_hresult'] == 0 and r['begin_before'] <= r['begin_after'] <= r['end_before'] <= r['end_after'] for r in submitted), 'update HRESULT/order')
        first = submitted[0]
        first_end = (first['end_before'] + first['end_after']) / (2 * qpc_frequency)
        first_end_times.append(first_end)
        for peak in result['streams'][stream]:
            lag = peak['peak_lag_frames']
            packet = next(p for p in packets if p['frame_offset'] <= lag < p['frame_offset'] + p['frames'])
            time = packet['qpc_100ns'] / 1e7 + (lag - packet['frame_offset']) / RATE
            peak['loopback_peak_qpc_seconds'] = time
            peak['peak_after_first_end_midpoint_ms'] = (time - first_end) * 1000
            peak['peak_after_clock_origin_ms'] = (time - origins[-1]) * 1000
        np.asarray(kernels[stream], dtype='<f8').tofile(str(output / ('matched_ir_stream%d.f64' % stream)))
    common = (kernels[0] + kernels[1]) * .5
    common.astype('<f8').tofile(str(output / 'matched_common_ir.f64'))
    observed_difference = [(result['streams'][1][ear]['loopback_peak_qpc_seconds'] - result['streams'][0][ear]['loopback_peak_qpc_seconds']) * 1000 for ear in range(2)]
    clock_difference = (origins[1] - origins[0]) * 1000
    packet_origins = [p['qpc_100ns'] / 1e7 - p['device_position'] / RATE for p in packets]
    steady_origins = [p['qpc_100ns'] / 1e7 - p['device_position'] / RATE for p in packets if not (p['flags'] & 1)]
    packet_future_ms = [(p['qpc_100ns'] / 1e7 - p['host_before'] / qpc_frequency) * 1000 for p in packets]
    tail = stereo[-min(len(stereo), RATE // 5):]
    result.update({
        'passed': True, 'status': 'identified', 'native_elapsed_seconds': native['elapsed_seconds'],
        'capture_frames': len(stereo), 'capture_peak': float(np.max(np.abs(stereo))),
        'capture_rms': np.sqrt(np.mean(stereo ** 2, axis=0)).tolist(),
        'final_200ms_rms': np.sqrt(np.mean(tail ** 2, axis=0)).tolist(),
        'final_200ms_peak': float(np.max(np.abs(tail))),
        'loopback_packet_origin_observed_span_us': (max(packet_origins) - min(packet_origins)) * 1e6,
        'loopback_steady_packet_origin_span_us': (max(steady_origins) - min(steady_origins)) * 1e6 if steady_origins else None,
        'loopback_packet_timestamp_minus_host_getbuffer_ms_median': float(np.median(packet_future_ms)),
        'clock_origin_qpc_seconds': origins, 'estimated_clock_origin_difference_ms': clock_difference,
        'post_start_query_separation_ms': (post_start_times[1] - post_start_times[0]) * 1000,
        'first_end_qpc_seconds': first_end_times,
        'first_end_call_midpoint_separation_ms': (first_end_times[1] - first_end_times[0]) * 1000,
        'observed_stream1_minus_stream0_output_peak_ms_per_ear': observed_difference,
        'output_difference_minus_clock_origin_difference_ms_per_ear': [value - clock_difference for value in observed_difference],
        'matched_ir_shape': [4160, 2], 'matched_ir_earliest_peak_index': 64,
        'matched_ir_includes_object_gain': True,
        'matched_ir_estimates_include_correlation_noise': True,
        'latency_scope': 'Current selected provider/native rendering/postmix and endpoint loopback path; not pure Sonic DSP or an acoustic measurement',
        'spatial_streams_started_by_analyzer': 0,
    })
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--self-test', action='store_true')
    parser.add_argument('--capture', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    if args.self_test:
        print(json.dumps(self_test(), indent=2)); return
    require(args.capture is not None and args.output is not None, '--capture and --output required')
    args.output.mkdir(exist_ok=False)
    before = {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in args.capture.iterdir() if p.is_file()}
    try:
        result = analyze(args.capture, args.output)
    except (ValueError, StopIteration, KeyError) as error:
        result = {'passed': False, 'status': 'inconclusive', 'diagnostic': str(error), 'spatial_streams_started_by_analyzer': 0}
    after = {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in args.capture.iterdir() if p.is_file()}
    require(before == after, 'raw capture changed')
    result['raw_files_unchanged'] = True
    (args.output / 'result.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, indent=2))
    if not result['passed']: sys.exit(1)


if __name__ == '__main__':
    main()
