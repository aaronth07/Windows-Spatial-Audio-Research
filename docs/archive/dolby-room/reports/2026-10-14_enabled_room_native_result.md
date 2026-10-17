> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Native Dolby room coefficient control verified

The native consumer uses the mapped room taps and feedback independently of the personalized directional coefficients. With fallback lane15689 held at0, zero output taps removes the added room return, while half complex feedback produces a shorter decay. Native object rendering and personalized directional processing remain active. The user's original low-reverb personalized configuration was restored exactly after every completed experiment.

Ten-position session, complete analysis, test plan. Ten captures completed: enabled twice, half-feedback twice, enabled-restored twice, muted-taps twice, original-personalized-restored twice. Each group contains the same ten object positions and unchanged stimulus/format/category. All100 positions completed; maximum PCM magnitude0.02463. All six saved endpoint/context registry keys were byte-identical afterward, with no notification-counter difference and unchanged selected/active Dolby format.

| Comparison against enabled reference | Direct5ms energy change, both ears |20–250ms energy change |
| --- | ---: | ---: |
| Half feedback |−0.00034..+0.00236dB|−7.950..−7.741dB|
| Enabled restored |−0.00100..+0.00331dB|−0.00129..+0.00678dB|
| Muted output taps |−0.00046..+0.00248dB|−22.447..−20.747dB|

Peak ITD differences were0 samples. Maximum direct-window ILD difference was0.00078dB for feedback and0.00112dB for muted taps. Maximum median absolute direct-spectrum difference was0.00796dB and0.01252dB respectively. Direct-window relative L2 differences were0.00035..0.00103 for feedback and0.00037..0.00119 for muted taps. These are compatible with the restored-reference repeatability, rather than a changed directional transfer.

Whole-response first8192-sample relative L2: enabled repeat0.002011, feedback repeat0.001826, enabled-restored repeat0.002072, muted repeat0.001787, personalized-restored repeat0.001781. Enabled→restored0.002700; enabled→feedback0.052686; enabled→muted0.089797. The change exceeds repeatability and reverses on restoration. One common stereo-field timing correction is used per comparison; no ear/direction normalization or alignment is applied.

Subtracting matched muted responses from enabled and feedback responses isolates the changed room contribution under the same source normalization, subject to measurement noise. Noise-compensated Schroeder integration uses2ms blocks, truncates at the last block10dB above the estimated floor, and fits−5..−25dB only when a sufficient interval exists. All ten fitted curves passed R²≥0.99317. RT60 estimated from this T20 fit was225.94..227.16ms for original feedback and125.34..127.17ms for half feedback. These describe this renderer-response diagnostic; they are not physical-room RT60 calibration or a promise that halving feedback halves decay time.250..900ms energies are near the floor and do not support a separate long-tail claim. Measurement plot.

The native parser returned success with intact scratch canaries for both candidates, unchanged PCA field hashes, unchanged matrix, and expected tap/feedback hashes. Candidate manifest, tap validation, feedback validation. The half-feedback contraction bound from the retained unit-norm matrix and quantized complex coefficients is0.391111; delays and allocation were unchanged.

The causal explanation is now bounded and repeatable: the persisted fallback1 suppresses profile1 room excitation and can close the final room gate; setting0 permits it. Room coefficient edits were consequently inaudible in earlier control1 tests because this room return was not contributing measurably. Once enabled, the same mapped taps/feedback produce the expected large output changes. Exact source-index creation on the Windows side remains unobserved, so this is a causal explanation for the measured configuration and test conditions, not every historical device switch or subjective observation. Native control mapping, source/send mapping, fallback A/B/A.

## Existing-stream and command verification

Continuous test kept one dynamic object and paired capture open across original→On→Off→HalfFeedback→Restore. Each phase's first sweep was a discarded transition guard; front/left/back/right were then compared. Control commands completed in0.890..1.093seconds within3.25-second guard intervals. All25 positions completed, with no app-requested pause/reopen/reset between groups. Analysis: all comparison lags0; original→restored L2 0.001622; On→Off0.082527; On→HalfFeedback0.048428. On→Off direct energy differed−0.000584..+0.000031dB while20–250ms energy fell20.750..22.056dB. This proves the property controls affect existing object playback, without claiming a sample-accurate switching latency.

The practical control cycle completed nine commands with exact expected profile states and no writes for repeated On/Restore. Every transition preserved the other saved configuration. Final raw registry equality is true. Set-DolbyRoom.ps1 provides Status/On/Off/HalfFeedback/Restore and durable backup handling.

Two harness issues were corrected and their failed attempts retained: an absent PowerShell optional path arrived as empty string, stopping before the first room-group write; the continuous event reader initially tried to parse a diagnostic line as JSON, stopping before any profile transition. Both attempts restored exact configuration. The continuous analyzer also required an explicit25-direction count rather than the older ten-direction default; raw recordings were retained and analyzed after that bounded fix. A baseline-specific getter test was initially run during the deliberate candidate state and correctly rejected its hash; the full suite must be run after restoration. These are not audio outcomes.

The original personalized baseline remains API `<private-artifact-digest>`, registry `<private-artifact-digest>`. No Access launch, provider toggle, default/routing/format/EQ/driver/service change occurred in these room experiments. Arbitrary measured-room FIR replacement remains outside the native topology demonstrated here.
