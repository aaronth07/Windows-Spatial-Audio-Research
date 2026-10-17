> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Personalized directional coefficients are used

The isolated half-gain diagnostic confirms that the current personalized directional coefficient fields reach native Dolby Atmos for Headphones object rendering. Across all ten tested directions, halving only those fields changed output energy by−6.0169..−6.0186dB. A single fitted common amplitude gain was0.5001346, with0.002897 relative residual. Restoring the exact original profile restored the native response.

This is stronger than property readback or native parser acceptance: the measured change matches the predicted directional-field contribution, with room, phase, geometry, matrices and runtime bytes retained. It does not establish acoustic/personal accuracy, a lossless recovered HRIR, or every other renderer branch.

Evidence: six-run manifest, untrimmed deconvolution and comparison, exact test plan, native decoder validation.

Baseline→restored relative L2 difference was0.00173285 over the first8,192samples, with0.00171489 residual after fitting one diagnostic common gain. Candidate repeat difference was0.00342063, as expected to be more noise-sensitive after halving signal level. No per-ear normalization/alignment or independent directional gain was used in the comparison. QPC transport timing and one common whole-field/stereo lag were retained.

The second baseline recording had a single transient in its final left-direction interval, reaching PCM0.385681 rather than the typical0.023. Its whole-field baseline repeat difference was0.0139206, dominated by that interval's0.03747 difference. The retained data do not identify the transient's originating process. It was not clipping, a native API error or an incomplete direction, and it is not evidence of a coefficient change. The decisive baseline→candidate and baseline→restored comparisons use the first baseline. An additional restored-baseline recording matched the first baseline at0.00170507 relative L2 difference, with maximum PCM0.0230103. Its result is preserved separately in additional baseline analysis. Raw/initial analysis records were retained unchanged.

The API/registry candidate identities were `<private-artifact-digest>` / `<private-artifact-digest>`. Both final restoration passes read back current personalized API `<private-artifact-digest>` and registry `<private-artifact-digest>`. All six endpoint/context registry keys are byte-identical to the fresh pretest snapshot; Dolby remains selected/active. No provider toggle, format/default/routing, Access launch, service/driver or other property change occurred.

Combined with the earlier tap-zero result, this rules out a blanket failure to consume the modified initialization's directional block under these stream conditions. The selected room structure, zero room send, distance/source-profile selection and room enable gate remain to be distinguished. An unused tap path does not imply that room replacement is impossible. The next static trace targets the known near-anechoic/source-profile runtime controls before another live field-group test.
