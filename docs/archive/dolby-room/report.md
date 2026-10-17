> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

Current findings and fresh verification: 2026-10-15 PHRTF room causality. The preliminary proposed test below was subsequently completed; see README.md for existing verified controls.

# Dolby headphone room-control result

The smallest complete, mapped room-output edit found in this bounded pass is zeroing 256 table-A output taps, plus repairing the global rp checksum. Native code maps those lanes to a room-specific output multiplier; the directional PCA table and its shared exponent remain unchanged. A separate offline candidate is ready for primary review. It has not been delivered to the consumer or called a complete room-off profile.

Parameter map links the exact lanes, API offsets, native fields, consumers, candidate hashes, tests, and obstacles. Native evidence retains eight focused shipped-code dossiers. Candidate manifest lists all files, hashes, allowed and actual byte changes. The API candidate is 62,791 bytes, SHA-256 `<private-artifact-digest>`.

The current low-reverb personalized source/API hashes were preserved. Eight script tests passed, including malformed inputs, overwrite/path refusal, byte-level isolation, command-line paths, and public-model room/direct separation. Candidate generation and the default inspector executed; both scripts compiled. In the public model, the selected room impulse becomes zero while tested directional gains and room sends remain unchanged. Native acceptance and all-path suppression remain untested.

Primary review found that the initial script evaluated union annotations during Python 3.8 import. Postponed annotations fixed that compatibility error without changing candidate bytes or calculations. All eight tests, including CLI generation/inspection, and compilation then passed with `<local-path>`. The initial run used the local Python 3.12 environment; the final compatibility run used the project's Python 3.8 runtime.

The actual O/P/O/P paired comparison remains the causal evidence: personalized 20–80 ms energy is 10.6–12.2 dB lower and 80–300 ms energy is 6.0–6.3 dB lower, while 300–1800 ms is at effectively unchanged recorded background. These are measured complete-tuning differences, not isolated room effects. The earlier statement that late tails match does not negate the early/intermediate reduction. Eight original paired WAVs/SOFAs were read and hashed in capture inventory; no later corrected/truncated 710-direction derivative was substituted for them. The later 37-direction phase/timing repair is separate from this causality question.

All live settings and the current profile were left untouched. No Access launch, endpoint/profile write, toggle, playback, capture, binary patch, or native DLL invocation occurred. REA was coordinated with the primary and closed after the focused static pass.

## Proposed current/candidate/current test

After primary review and explicit live authorization, use the exact current low-reverb personalized API `94b3df...` as the baseline and restoration state. Do not restore the 14-byte original unpersonalized payload.

1. Read exact current API/registry hashes, active Atmos format, defaults, formats, and five other User values. Retain the immediate current-profile backup. Stop only experiment-owned streams before any authorized property operation.
2. Record two current-profile runs, then two candidate runs, then two restored-current runs. Use the same existing single dynamic object, SoundEffects category, six cardinal unit-radius positions with repeated front/left, paired selected virtual endpoint, 48 kHz, one-second ESS at −34 dBFS, 0.25-second pre-silence and two-second tails. Reuse stimulus/inverse/grid bytes. Pin expected API hash per state and verify after stopping.
3. Delivery must use a separately reviewed single-property API writer supporting the exact new hash and current restore hash. Save readback and other-value hashes. No Access launch or format toggle is in this plan. If fresh streams plus property notifications do not establish uptake, stop and report the ambiguity; a format cycle would require a separately justified, authorized experiment.
4. Use an outer cleanup that restores the exact current-profile backup, checks current API/registry hashes, and verifies other values/defaults/formats. A failure rollback to any old unpersonalized baseline is not acceptable.
5. Compare within-state repeatability and restored-current agreement before causal interpretation. Retain absolute gain/full response and shared stereo timing. Quantify early, 20–80, 80–300, and 300–1800 ms energy with the measured floor. Do not use independent ear shifts, normalization, shortening, or derivative truncation as room-off evidence. Confirm an effect exceeds within-state variation. Unchanged output can indicate alternate room selection, ignored candidate, or weak/no feed; it cannot prove the tap map is wrong or room is already absent.

Remaining decisions: native decode acceptance, actual endpoint runtime overrides/alternate room path, and perceptual benefit. The primary retains acceptance and project strategy.
