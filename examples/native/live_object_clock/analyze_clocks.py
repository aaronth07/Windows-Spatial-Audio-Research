"""Analyze retained clock pairs offline; does not create an audio stream."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import statistics


def percentile(values, fraction):
    ordered = sorted(values)
    index = (len(ordered) - 1) * fraction
    lower = int(index)
    upper = min(lower + 1, len(ordered) - 1)
    return ordered[lower] + (ordered[upper] - ordered[lower]) * (index - lower)


def analyze(raw, carrier):
    assert raw['host_qpc_frequency'] > 0 and raw['clock_log_hresult'] == 0
    assert len(raw['streams']) == 2 and len(raw['records']) <= 1024
    results, normalized = [], []
    for service in raw['streams']:
        index = service['producer_stream']
        assert service['clock_interfaces_released'] == 1
        assert service['clock_service_hresult'] == service['frequency_hresult'] == 0
        frequency = service['clock_frequency']
        assert frequency > 0
        records = [r for r in raw['records'] if r['producer_stream'] == index]
        assert records and records[0]['phase'] == 0 and records[1]['phase'] == 1
        assert sum(r['phase'] == 0 for r in records) == sum(r['phase'] == 1 for r in records) == 1
        assert all(r['position_hresult'] == 0 for r in records)
        assert all(r['host_qpc_before'] <= r['host_qpc_after'] for r in records)
        assert all(b['position'] >= a['position'] and b['position_qpc_100ns'] >= a['position_qpc_100ns'] for a, b in zip(records, records[1:]))
        host_frequency = raw['host_qpc_frequency']
        bracket_errors = [r for r in records if not (r['host_qpc_before'] * 1e7 / host_frequency <= r['position_qpc_100ns'] <= r['host_qpc_after'] * 1e7 / host_frequency)]
        assert not bracket_errors, 'correlated QPC outside measured call bracket'
        if service['clock2_service_hresult'] != 0:
            assert all(r['device_hresult'] == service['clock2_service_hresult'] for r in records)
            assert all(r['device_position'] == r['device_qpc_100ns'] == 0 for r in records)
        updates = {}
        for record in carrier['records']:
            if record['stream_id'] == index + 1:
                updates[record['update']] = max(updates.get(record['update'], 0), record['frames'])
        previous, pending_frames = 0, None
        for record in records:
            phase, update = record['phase'], record['update']
            if phase < 2:
                assert update == 0 and record['submitted_update_frames'] == 0
            elif phase == 2:
                assert update == previous + 1 and record['submitted_update_frames'] == previous * 480
                pending_frames = record
            else:
                assert phase == 3 and pending_frames is not None and update == pending_frames['update']
                # The observed native stream used 480 frames for every update,
                # including the final empty EOS drain update.
                assert record['submitted_update_frames'] - pending_frames['submitted_update_frames'] == 480
                previous, pending_frames = update, None
            payload_frames = sum(frames for ordinal, frames in updates.items() if ordinal < update or (ordinal == update and phase == 3))
            position_frames = record['position'] * 48000 / frequency
            normalized.append({
                'producer_stream': index, 'phase': phase, 'update': update,
                'completed_update_frames': record['submitted_update_frames'],
                'submitted_payload_update_frames': payload_frames,
                'stream_position_seconds': record['position'] / frequency,
                'stream_position_48k_equivalent_frames': position_frames,
                'payload_counter_minus_clock_equivalent_frames': payload_frames - position_frames,
                'position_qpc_100ns': record['position_qpc_100ns'],
            })
        assert pending_frames is None and previous == len(updates) + 1
        nonzero = [r for r in records if r['position'] > 0]
        assert len(nonzero) > 10
        origins = [r['position_qpc_100ns'] / 1e7 - r['position'] / frequency for r in nonzero]
        origin = statistics.median(origins)
        first_time = nonzero[0]['position_qpc_100ns']
        first_position = nonzero[0]['position']
        x = [(r['position_qpc_100ns'] - first_time) / 1e7 for r in nonzero]
        y = [r['position'] - first_position for r in nonzero]
        mean_x, mean_y = statistics.mean(x), statistics.mean(y)
        slope = sum((a - mean_x) * (b - mean_y) for a, b in zip(x, y)) / sum((a - mean_x) ** 2 for a in x)
        rate_error_ppm = (slope / frequency - 1) * 1e6
        assert abs(rate_error_ppm) < 10000, 'rate inconsistent with reported frequency'
        durations = [(r['host_qpc_after'] - r['host_qpc_before']) * 1e6 / host_frequency for r in records]
        post_start = records[1]['position_qpc_100ns'] / 1e7
        results.append({
            'producer_stream': index, 'get_service_hresult': service['clock_service_hresult'],
            'clock2_get_service_hresult': service['clock2_service_hresult'],
            'clock_frequency_units_per_second': frequency,
            'position_records': len(records), 'nonzero_position_records': len(nonzero),
            'all_position_hresult_s_ok': True, 'position_and_qpc_monotonic': True,
            'position_qpc_within_host_call_bracket': True,
            'position_before_and_after_start': [records[0]['position'], records[1]['position']],
            'post_start_position_qpc_seconds': post_start,
            'estimated_nonzero_clock_origin_qpc_seconds': origin,
            'origin_min_qpc_seconds': min(origins), 'origin_max_qpc_seconds': max(origins),
            'observed_origin_span_microseconds': (max(origins) - min(origins)) * 1e6,
            'estimated_origin_after_post_start_query_ms': (origin - post_start) * 1000,
            'fitted_rate_units_per_second': slope, 'short_trial_fitted_rate_error_ppm': rate_error_ppm,
            'clock_pair_call_us_p99': percentile(durations, .99), 'clock_pair_call_us_max': max(durations),
            'completed_update_frames_final': records[-1]['submitted_update_frames'],
            'submitted_payload_update_frames_final': sum(updates.values()),
        })
    assert len(set(s['producer_stream'] for s in results)) == 2
    a, b = sorted(results, key=lambda s: s['producer_stream'])
    estimate = (b['estimated_nonzero_clock_origin_qpc_seconds'] - a['estimated_nonzero_clock_origin_qpc_seconds']) * 1000
    return {
        'passed': True, 'raw_clock_records': len(raw['records']), 'streams': results,
        'post_start_query_separation_ms': (b['post_start_position_qpc_seconds'] - a['post_start_position_qpc_seconds']) * 1000,
        'estimated_stream0_progress_ahead_ms': estimate,
        'estimated_stream0_progress_ahead_48k_frames': estimate * 48,
        'observed_origin_difference_range_ms': [
            (b['origin_min_qpc_seconds'] - a['origin_max_qpc_seconds']) * 1000,
            (b['origin_max_qpc_seconds'] - a['origin_min_qpc_seconds']) * 1000,
        ],
        'clock2_hardware_position_available': all(s['clock2_get_service_hresult'] == 0 for s in results),
        'shared_hardware_position_origin_verified': False,
        'first_payload_presentation_timestamp_verified': False,
        'counter_minus_clock_is_queue_occupancy': False,
    }, normalized


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--capture', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(exist_ok=False)
    raw_bytes = (args.capture / 'clocks.json').read_bytes()
    carrier_bytes = (args.capture / 'capture.json').read_bytes()
    raw, carrier = json.loads(raw_bytes), json.loads(carrier_bytes)
    summary, normalized = analyze(raw, carrier)
    failures = []
    for name in ('zero_frequency', 'position_regression', 'qpc_regression', 'failed_hresult', 'bad_submission_counter', 'truncated_record_pairs'):
        damaged = copy.deepcopy(raw)
        selected = next(r for r in damaged['records'] if r['producer_stream'] == 0 and r['update'] == 50 and r['phase'] == 2)
        if name == 'zero_frequency': damaged['streams'][0]['clock_frequency'] = 0
        elif name == 'position_regression': selected['position'] = 1
        elif name == 'qpc_regression': selected['position_qpc_100ns'] = 1
        elif name == 'failed_hresult': selected['position_hresult'] = 0x80004005
        elif name == 'bad_submission_counter': selected['submitted_update_frames'] += 1
        else: damaged['records'].pop()
        rejected = False
        try:
            analyze(damaged, carrier)
        except (AssertionError, ValueError, ZeroDivisionError):
            rejected = True
        assert rejected, name
        failures.append({'name': name, 'rejected': True})
    summary['negative_cases'] = failures
    summary['raw_sha256'] = hashlib.sha256(raw_bytes).hexdigest()
    assert raw_bytes == (args.capture / 'clocks.json').read_bytes() and carrier_bytes == (args.capture / 'capture.json').read_bytes()
    summary['raw_files_unchanged'] = True
    summary['spatial_streams_started_by_analyzer'] = 0
    (args.output / 'result.json').write_text(json.dumps(summary, indent=2) + '\n')
    (args.output / 'normalized_pairs.json').write_text(json.dumps(normalized, indent=2) + '\n')
    print(json.dumps(summary, indent=2))


if __name__ == '__main__':
    main()
