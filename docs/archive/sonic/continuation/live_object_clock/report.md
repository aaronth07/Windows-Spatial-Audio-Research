> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Spatial stream clock correlation

Actual GetService(IAudioClock) succeeded on both Windows spatial streams. GetFrequency returned 384,000 units/second. All 408 GetPosition calls returned S_OK, and their positions and correlated QPC values were monotonic. Actual GetService(IAudioClock2) returned E_NOINTERFACE (`0x80004002`) on both streams. Direct device-frame positions therefore remain unavailable through the requested service on this endpoint.

The QPC-correlated stream clocks had different inferred origins: stream 0 was approximately **11.623783 ms ahead** of stream 1 at a common QPC time, equivalent to 557.942 frames of 48 kHz duration. The post-Start clock queries were only 0.100200 ms apart. Close Start calls did not establish equal stream-relative playback progress.

| Measurement | Producer stream 0 | Producer stream 1 |
|---|---:|---:|
| Clock service / frequency HRESULT | S_OK / S_OK | S_OK / S_OK |
| Clock2 service HRESULT | E_NOINTERFACE | E_NOINTERFACE |
| Position records / nonzero records | 204 / 200 | 204 / 198 |
| Position before Start / just after Start | 0 / 0 | 0 / 0 |
| Estimated nonzero clock origin, QPC seconds | 358730.579834467 | 358730.591458250 |
| Observed origin span | 46.433 us | 58.433 us |
| Origin after post-Start query | 7.184367 ms | 18.707950 ms |
| Fitted rate | 383999.409 units/s | 383998.886 units/s |
| Short-trial fitted rate error | -1.538 ppm | -2.901 ppm |
| Clock-call bracket p99 / maximum | 0.697 / 3.5 us | 1.894 / 1.9 us |

For each nonzero sample, the analysis computes `origin = position_qpc_100ns / 10^7 - position / GetFrequency()`. The reported origin is its median. The two observed origin ranges yield a difference range of 11.578700–11.683567 ms. This is the observed model spread, not a calibrated uncertainty bound. Fitted rate errors over approximately one second describe these readings; they do not establish long-term hardware drift. The zero readings around Start were excluded from origin/rate fits because initial clock stalling is documented behavior.

IAudioClock positions are stream-relative and meaningful only with their reported frequency. The position-QPC value uses 100 ns units; raw host QPC ticks are converted using the recorded QueryPerformanceFrequency, which was 10,000,000 here. Every correlated position timestamp fell within its measured host call bracket. Microsoft GetPosition documentation (historical reference unavailable)

GetService publicly lists IAudioClock2, but this actual endpoint's returned HRESULT is retained rather than substituting another position. Clock2 documents an unprocessed shared-mode device-frame position; those values cannot be inferred from the unsupported call's zero-initialized output fields. GetService (historical reference unavailable)

## Raw records and submitted frames

`live_run/clocks.json` retains the service/frequency HRESULTs, frequency and interface-release flags for each producer stream. Its 408 records contain the producer stream ordinal, phase, update ordinal, completed update-frame counter, GetPosition HRESULT/position/QPC, Clock2 HRESULT/output fields and raw host QPC before/after both calls. Phase names are before Start, after Start, after successful Begin and after End. All storage is preallocated; interfaces are obtained before Start and released on the same MTA thread after Stop/Reset. The clocks' methods are called through actual Windows service interfaces; the tap still forwards its actual stream GetService call.

The raw field `submitted_update_frames` counts full extents of successfully completed update transactions. It includes the final empty EOS drain transaction, so its final value is 48,480 while actual payload updates cover 48,000 frames. `clock_analysis/normalized_pairs.json` supplies the literal derived fields `completed_update_frames` and `submitted_payload_update_frames` separately. The original raw names and values are preserved.

Map `producer_stream + 1` to the controlled producer's sequential stream ID and join each clock update ordinal to the source carrier's update. Position divided by frequency maps observed stream progress to seconds at a common QPC time. It does **not** timestamp a particular GetBuffer payload, identify a next-write cursor, or measure queue occupancy. The counter-minus-clock values retain startup/render pipeline and cursor-offset uncertainty. The first payload's presentation time, a shared hardware position origin, native convolution latency and cross-stream sample-exact output alignment are not verified by these clock calls. A timestamped rendered-output measurement is needed for the payload-to-output timing relationship.

## Verification and scope

One bounded actual two-stream trial passed in 1.1041655 seconds. The preserved object-reuse capture still passed 400 records / 192,000 samples and exact metadata reference comparison; both streams had final inactive objects, successful Stop/Reset, balanced proxy releases and restored producer imports. The host counted zero instrumented operator new/delete calls during updates; this does not cover other DLL runtimes, malloc or HeapAlloc. Stream/configuration scope remains the reviewed interleaved producer, with no game injection or endpoint/default/provider/registry/service changes.

The final build passed MSVC C++17 `/O2 /MT /W4 /WX`. Nineteen native and ten carrier/offline cases passed. `clock_analysis/result.json` additionally rejects zero frequency, position regression, QPC regression, failed position HRESULT, incorrect update counter and truncated Begin/End record pairs. The second code pass checked HRESULT retention, 100 ns/QPC units, bounded clock storage, phase/counter ordering, output of unsupported service fields, same-thread interface release and preservation of the original capture/lifecycle path. The source uses a single producer thread for the clock log; concurrent clock-log appends are outside this proof.

From this directory, with Windows x64, Visual Studio C++ tools, Windows SDK and Python 3.8+:

Historical workspace command omitted; use the public [example guide](../../../../../examples/README.md).

Both Python tools use only the standard library. Output directories reject overwrite. The prior two-stream folder remains unchanged; `manifest.json` identifies this separate implementation and evidence.
