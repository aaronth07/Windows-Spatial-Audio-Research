> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Dolby headphone room control

Run `Set-DolbyRoom.ps1` in x64 PowerShell7. These controls use the current personalized profile on selected virtual endpoint with Dolby Atmos for Headphones already selected.

Historical workspace command omitted; use the public [example guide](../../../examples/README.md).

| Mode | Effect |
| --- | --- |
| On | Enable the profile's native room return |
| Off | Mute its room output taps while retaining the same direct-path normalization as On |
| HalfFeedback | Enable the room with half its complex feedback coefficients; measured shorter decay |
| Restore | Restore the exact saved low-reverb personalized profile |
| Status | Read the active profile state |

On/Off/HalfFeedback preserve the personalized directional coefficients. Compared with the original low-reverb setting, their shared source normalization slightly lowers direct output, about0.05–0.07dB in the tested positions. Their mutual direct-response differences were within measurement repeatability. HalfFeedback is a coefficient setting, not a calibrated decay-time input.

The command saves durable backups under `controls/backups` before its first change and retains the backup pointer for Restore. Keep that directory. It checks the exact previous profile and all other Dolby User properties, refuses unrelated drift, and never selects a spatial provider. The current selected virtual endpoint endpoint, profile and consumer version are intentionally pinned; these are local research controls. Close Dolby Access during changes. No automatic Atmos toggle or audio-service restart is used.

Native verification: ten-position coefficient-group recordings, exact restoration, and On/Off/HalfFeedback/Restore within one continuously open dynamic-object stream all passed. The practical nine-command control cycle also passed, including repeat-command idempotency. Results, continuous-stream evidence, control cycle.

The payload exposes a constrained subband feedback network. These controls adjust that native room; importing an arbitrary measured room FIR or directional BRIR set is not implemented. Representation limits. Personalized coefficient/SOFA reconstruction findings remain separate in `../DolbyCoefficientResearch`.
