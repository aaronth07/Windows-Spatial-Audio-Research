> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Native room output-tap comparison

The isolated room output-tap edit did not produce a measurable response change beyond repeatability in the current personalized Dolby configuration. This does not establish a working room-off control or prove that the active renderer consumed the changed block.

Experiment: retained six-run manifest, analysis, exact plan. Sixty native dynamic-object directional sweeps completed through selected virtual endpoint/selected virtual endpoint. Maximum recorded PCM magnitude was0.0230103; capture reported no discontinuity, clipping or incomplete direction.

| Comparison | Relative L2 difference, first8,192 IR samples |
|---|---:|
| Baseline repeat |0.00175455|
| Candidate repeat |0.00177263|
| Restored repeat |0.00174894|
| Baseline→restored |0.00173265|
| Baseline→muted taps |0.00171851|

QPC timing was retained; one common whole-field/stereo lag removed differences of9–162samples across comparisons. No ear-specific shift or normalization was used. These lags represent capture/processing transport variation, not changed interaural timing. Peak ITD changes were zero for all ten directions. Direct-window median spectral magnitude differences were0.0016–0.0074dB, and20–250ms energy changes were−0.0117..+0.0109dB. The comparable repeat differences prevent assigning these small deviations to the room edit. The first5ms window is a time region and may include overlapping room energy.

The property controller read back the exact62,791-byte candidate API SHA-256 `<private-artifact-digest>` and its registry representation. Every other Dolby User property matched. No provider, format, routing or default endpoint change occurred. Before final captures and again in outer cleanup, the current personalized API `<private-artifact-digest>` was restored. All six saved endpoint/context registry keys match byte-for-byte, with no notification-sequence difference. The selected/active format remains Dolby.

Subsequent isolated native parser execution adds a separate finding: the pinned consumer DLL's size validator0x18010dab0 and fast deserializer0x18010fd30 return0 for baseline and candidate; both need67,032scratch bytes and retain their allocation canaries. The candidate decodes to zero native output taps, while both decoded directional-field arrays have identical baseline/candidate hashes and exact parity with the inspected logical fields. The decoded room+0x7c value is64 in both. Candidate native decode, baseline decode. This is parser acceptance and field isolation, not active stream uptake.

The native checksum branch accepts core or extended checksums; the owned/current and candidate extended checksums therefore do not fail solely because the core-only checksum differs. Remaining explanations include configuration not applied to the processing instance, configured-vs-default fallback, a different selected room structure, zero room send, or a downstream room gate. Next test: a separately isolated half-gain directional-field diagnostic, preserving room data, to distinguish coefficient uptake from the room-path alternatives. Its exact restoration target remains the current personalized profile.
