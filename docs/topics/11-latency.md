# Latency, clocks and real-time evidence

Separate packet duration, filter buffering, processing time, and device delay before comparing latency results. The table keeps those measurements separate from scheduling, queue residence, and the inverse filter’s target delay.

| Quantity | Example at 48 kHz | Interpretation |
|---|---:|---|
| 128 frames | 2.667 ms | Custom kernel quantum/buffering |
| 480 frames | 10 ms | Observed public update/callback extent in some trials |
| 1024 frames | 21.333 ms | Tested private frontend/source codec packet extent |
| 480-frame inverse target delay | 10 ms | Deliberate XTC design parameter |
| A sub-millisecond native call | CPU/wall observation | Not end-to-end latency |

## Stream clocks

Actual IAudioClock services succeeded in the tested spatial streams with frequency 384,000 units/second. Positions must be divided by GetFrequency, not assumed to be sample frames. The correlated position timestamp uses 100 ns QPC units; host QueryPerformanceCounter ticks need their own frequency conversion.

IAudioClock2 was listed by the public service contract but returned E_NOINTERFACE on the tested endpoint. A missing service output must not be replaced by zero-initialized fields and interpreted as a real device position.

One trial inferred approximately 11.624 ms difference between stream clock origins despite near-simultaneous starts. A subsequent different trial found approximately equal origins. A constant startup offset cannot be reused across sessions. A clock origin also does not establish which GetBuffer payload is being presented, a next-write cursor or queue occupancy.

## Actual loopback identification

Two independent low-level PN sources at the same position were rendered through real spatial streams and captured in same-endpoint WASAPI loopback. Both source signatures were identifiable, and their peaks occurred at the same capture sample in both ears. The matched estimates retained one common shift per stereo pair.

API timestamps placed the peak 26.367–26.456 ms after the first EndUpdatingAudioObjects call midpoint. This includes the currently selected provider, native processing, postmix and virtual endpoint path. It was not a selected-Sonic-only experiment or the custom sixteen-bus output chain.

The virtual endpoint's packet timestamps were ahead of host packet retrieval. Startup carried a discontinuity flag; the identified peak packet was clean. Therefore this is an API-timestamp-based loopback result, not independently calibrated acoustic/hardware arrival. The within-packet relative peak equality avoids that shared absolute timestamp offset.

Finite PN cross-correlation estimates retain correlation noise; they are not exact arbitrary-long impulse-response deconvolutions. The analyzer rejects silence, correlated sources, unrelated noise, invalid timestamps and inadequate peak/background separation instead of inventing latency from a weak signal.

## Queue and deadline tests

A demand-driven local-file decoder can read ahead and follow the device consumer. A network/live source may have an independent clock. Even a small sustained clock-rate mismatch eventually starves or overruns a bounded queue without rate control.

Measured schedules and injected stalls test specific queue hypotheses. They do not establish immunity to unbounded scheduling delay. Keep underrun counts, valid frame counts, padding, source index and reset/discard behavior in the transport contract.

Paced convolution runs should distinguish late wake-up from processing-budget overrun. Instrumented operator-new counts cover the tested module's operators, not every allocation in Windows/vendor DLLs. Long-load and actual device tests remain necessary before claiming game-ready end-to-end behavior.
