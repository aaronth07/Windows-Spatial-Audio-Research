> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Room output taps across five object distances

The tap-zero edit again produced no repeatable response change attributable to the coefficient modification. The result now covers front/left objects at0.125,0.25,1,4,16metres. The Windows position input is measured in metres; no internal Dolby distance/profile equivalence is inferred.

Six-run manifest, initial analysis, second-baseline reference analysis, test plan. All sixty positions completed, maximum measured PCM0.0271, with no reported clipping or capture discontinuity. Both restoration passes returned current personalized94b3…5ff4; all six endpoint/context keys matched byte-for-byte and Dolby remained selected/active.

The first baseline contains a nonrepeatable480-sample timing step after its sixth position: direct peaks change from1534 to2014 samples. Its second baseline, both candidate captures and both restored captures have one constant direct-peak time across their ten positions. Submission-QPC correction returned zero offsets and capture flags reported no gap, so this step is not explained by the retained timestamp correction. Its origin within the renderer/acquisition chain remains unknown. A first-baseline whole-field waveform comparison consequently gives a large error and is unsuitable for attributing a profile effect. That initial analysis and all raw data remain unchanged.

The second baseline is used in a separately named analysis, without per-direction or per-ear shifts. Candidate repeat relative L2 difference is0.00147372 and restored repeat0.00149780. Clean baseline→candidate difference is0.00284876, comparable with clean baseline→restored0.00302463. Energy differences are approximately0.002dB for both changes. The comparable restored difference and matched candidate/restored behavior do not support a room-tap effect. Nothing was independently normalized to obtain that result.

This rules out the one-metre input radius as the sole explanation for the original null tap comparison over this tested range. It does not establish which internal room structure/source profile is active. The directional half-gain test independently established consumption of the personalized directional fields. A new static lead is the serialized trailer fallback control used by0x180018b70: the final coefficient words are[0,1,1], and the middle word is a candidate for the PCA+0x15c control that permits room output only when zero after its counter threshold. Exact mapping/other consumers are being checked before any proposed live edit.
