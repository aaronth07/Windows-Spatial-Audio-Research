> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Checkpoint — 2026-10-13 15:10 UTC

The six-run native validation completed successfully from 15:02:34.1055429 to 15:03:55.7500375 UTC; the runner exited 0. The exact reviewed candidate property was set, committed, read back, then restored and verified. Candidate API/registry readbacks had SHA-256 `<private-artifact-digest>` / `<private-artifact-digest>`. Restored API/registry values matched the fresh baseline at `<private-artifact-digest>` / `<private-artifact-digest>`; all set and commit HRESULTs were S_OK.

Six current registry User values and app files matched the fresh backup. The endpoint inventory and console/multimedia defaults were unchanged. Atmos was off and active/default spatial GUIDs were zero at 15:03:54.9919007 UTC. Cleanup completed (`cleanup_incomplete=false`). No further live setting change or test was performed after the sequence.

All six native-object playback/capture runs returned S_OK. The input was mono float32, 48 kHz, one dynamic object. The paired recording stream captured stereo float32 at 48 kHz. Persisted playback and recording formats remained stereo 48 kHz PCM16 and PCM24, respectively. The native candidate produced repeatable differences at all five positions: front 0.244751, left 0.392137, right 0.384137, rear 0.232274, and above 0.340573 relative waveform difference. Baseline/restored variation was zero; the candidate repeat error was zero after one shared stereo lag of −122 samples for candidate1 and 0 for candidate2. Across 502,740 aligned frames, independent whole-capture comparison found exact sample equality for baseline2 and both restored captures, exact equality between candidate repeats, and candidate difference from controls.

This is evidence that the PHRTF property change caused reproducible changed native processing in this tested Windows Spatial Sound path. Direct full internal coefficient decode or room/PCA pointer consumption remains inferred from static code. This does not show Dolby Access UI/cache integration, restart persistence, persistent enablement, or acoustic/perceptual correctness. The comparison's fivefold-control threshold is an operational discriminator, not a statistical probability.

Primary evidence copied into this dated report directory and hash-verified:

- `native_load_validation_manifest_20261013T150234.json` — 7,837 bytes, SHA-256 `<private-artifact-digest>`.
- `native_load_capture_comparison_20261013T150234.json` — 4,730 bytes, SHA-256 `<private-artifact-digest>`.
- `native_load_verification_20261013.json` — 21,803 bytes, SHA-256 `<private-artifact-digest>`.

Original validation records and private captures remain preserved. No approved sources, runtime proposals, or other live files were changed for this report. Next step: continue the primary agent's static metadata tracing; no further live retest is planned.
