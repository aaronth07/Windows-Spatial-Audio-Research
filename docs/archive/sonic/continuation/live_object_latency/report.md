> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Spatial source to endpoint loopback timing

One bounded live trial identified both independent spatial source signals in actual same-endpoint WASAPI loopback. Their output peaks occurred at the **same capture sample in each ear**. The API-timestamp-based peak latency from the first EndUpdatingAudioObjects call midpoint was **26.367–26.456 ms**. This measures the current selected provider/native rendering, postmix and endpoint loopback path; it does not isolate Sonic DSP or measure acoustic arrival.

The trial used two actual Windows spatial streams, one object each, with the same fixed Windows position `(0,0,-1)` and nominal volume 0.001. Each sent 0.2 seconds of independent MT19937 PN at raw amplitude at most 0.01, followed by 0.5 seconds of zeros and EOS. The same uniquely resolved selected virtual endpoint endpoint supplied stereo float32/48 kHz shared-mode loopback. No provider, endpoint default, routing, profile, master configuration, registry or service settings were changed by this probe. All files remain local to this folder; earlier capture/clock folders are preserved.

| Observation | Result |
|---|---:|
| Live capture interval | 1.2111476 s |
| Captured PCM / packets | 52,800 stereo frames / 110 |
| Valid frames per source / final inactive objects | 33,600 / one each |
| PN reference length / zero-input tail per source | 9,600 / 24,000 frames |
| Maximum normalized cross-correlation between source PN sequences | 0.033160 |
| Source 0 / 1 left-ear peak lag | 852 / 852 frames |
| Source 0 / 1 right-ear peak lag | 856 / 856 frames |
| Normalized source/output peak correlations | 0.3005–0.3578 |
| Peak / off-peak correlation RMS | 43.20–44.98 dB |
| Loopback peak amplitude | 2.5298e-5 |
| Final 200 ms RMS, left / right | 1.4824e-12 / 1.4907e-12 |
| Capture and both source Stop / Reset | S_OK / S_OK |
| Host loop instrumented operator new / delete | 0 / 0 |

`analysis_final/result.json` is the final analysis and `review_complete/result.json` is the final review. Earlier analysis/review directories retain superseded intermediate outputs. `live_run/result.json` is the unchanged native result. The live test was run once; the remaining tests only reread files or run the native self-test without opening an audio stream.

## Alignment and clocks

Actual IAudioClock services and GetFrequency succeeded on both streams, again returning 384,000 units/second. IAudioClock2 GetService returned E_NOINTERFACE on both. Each stream's nonzero GetPosition/QPC pairs gives an estimated stream-relative clock origin. This trial's origin difference was **0.000833 ms**, approximately 0.04 frames of 48 kHz duration, compared with an observed output-peak difference of **zero frames**. Post-Start queries were 0.096100 ms apart; first End-call midpoints were 0.006200 ms apart in the opposite order.

The earlier two-object clock trial estimated an 11.624 ms origin difference. This one-object trial does not reproduce that offset. The workloads and startup phases differ, and these observations do not identify which startup detail caused it. A constant offset cannot be reused across sessions. In this measured trial the output signatures coincide at capture-sample resolution; no general cross-stream sample alignment guarantee follows.

Each peak's output time is the GetBuffer packet QPC timestamp plus its sample offset within that packet. Each source's first End-call time is the midpoint between the recorded QPC values immediately before and after the actual call. The resulting latencies are:

| Source | Left peak after first End midpoint | Right peak after first End midpoint |
|---|---:|---:|
| 0 | 26.366850 ms | 26.450183 ms |
| 1 | 26.373050 ms | 26.456383 ms |

Relative to the inferred progressing stream-clock origins, the peaks are approximately 17.751–17.835 ms later. Those values retain the uncertainty of associating stream clock zero with the first payload. The observed output peak is not an onset estimate or an isolated convolution delay.

Packet 0 carries the permitted startup DATA_DISCONTINUITY flag; its QPC/device-position origin is about 1.196 ms from the steady median. All following packets have continuous device positions, zero flags and only 27.2 us observed origin span. Both peaks occur in packet 1, which has valid timestamps and no discontinuity. The API packet timestamps are a median 8.2453 ms ahead of the host GetBuffer retrieval time on this virtual endpoint. Consequently the table is explicitly based on the API's assigned frame timestamps, not an independent hardware presentation measurement. Relative stream peak equality is within the same packet and avoids that common timestamp offset.

WASAPI GetBuffer supplies the device-frame position and 100 ns QPC timestamp of a packet's first frame, together with discontinuity/timestamp-error flags. The native code copies the whole packet before ReleaseBuffer. Microsoft GetBuffer documentation. The Python analyzer independently reproduces all 19,200 reference samples from those seeds and checks the raw source files exactly. This avoids the affine LCG signals used in earlier capture-only trials. Each PN is correlated independently against each loopback ear. Only lags containing the full PN are searched. A result requires finite/nonzero data, normalized peak correlation at least 0.08, peak-to-off-peak RMS at least 10 dB, and ear agreement within two samples on the stream lag difference. Unidentifiable signals produce an inconclusive result instead of a latency claim.

`analysis_final/matched_ir_stream0.f64`, `matched_ir_stream1.f64` and `matched_common_ir.f64` contain little-endian float64 arrays shaped `[4160,2]`. They are matched-filter impulse response estimates: cross-correlation divided by PN energy. Their coefficients include the applied object volume and the current rendering path; no per-ear or per-source normalization is applied. One common shift per stereo pair preserves relative ear timing. Reference capture lag 852 maps to array index 64, with the right-ear peak at index 68. To restore the measured capture timeline, place the arrays' first sample at capture frame `852 - 64 = 788`.

The first 512 samples after the earliest peak, with 64 preceding samples, have cosine similarity 0.8919 left and 0.8860 right between the independent estimates. This supports a common response shape in this trial. The finite PN and simultaneous second source leave correlation noise in these estimates; they are not exact long-FIR deconvolutions. The full raw loopback and timestamps remain available for further analysis. The final measured tail approaches the numerical floor, but no claim is made about an unmeasured arbitrarily long response.

## Verification and reproduction

The x64 native build passed MSVC C++17 `/O2 /MT /W4 /WX`. Five native offline checks cover MT19937's known standard sequence, PN bounds, distinct sources, zero-lag cross-correlation and duration bounds. Nine numerical cases cover known synthetic delays/common response, relative stereo timing preservation, and rejection of silence, correlated sources, unrelated noise and nonfinite data. Eight additional review cases reject existing output, missing arguments, truncated PCM, timestamp errors, device-position gaps, QPC regression, failed clock HRESULT and corrupted source data. See `review_complete/result.json`.

The second code pass checked COM cleanup before CoUninitialize, same-thread service releases, GetBuffer/ReleaseBuffer ordering, event-handle lifetime, capture/frame bounds, silent-buffer handling, source EOS, capture timestamps, preserved stereo IR timing and output overwrite refusal. All storage is prepared before the live loop and files are written afterward. Allocation counts cover only the host's instrumented C++ operators, not Windows DLL allocations, malloc or HeapAlloc.

With Windows x64, Visual Studio C++ tools, Windows SDK, Python 3.8+, NumPy and SciPy, run from this folder. New output names are required.

Historical workspace command omitted; use the public [example guide](../../../../../examples/README.md).

This probe uses actual SDK interfaces directly; it does not inject into a game or replace the Windows provider. The loopback's current postmix and device path remain part of the measured response. There is no acoustic cancellation, selected-Sonic-provider or pure-Sonic-latency claim.
