> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

This is a proposed bounded test, not an executed import. It tests whether DTS uses the converted SOFA FIRs through native dynamic Windows spatial objects, then restores the exact prior state. The original prompts require preserving configuration and presenting exact live changes first. No global-default selection is included.

## Exact target and changes

Render endpoint: `{<identifier-removed>}` (selected virtual endpoint). Paired recording endpoint: `{<identifier-removed>}` (selected virtual endpoint). Every stream and format operation names these endpoints explicitly. The current global-default mismatch is not repaired.

1. Temporarily select DTS Headphone:X format `{<identifier-removed>}` on the render endpoint using the existing pinned SoundVolumeView executable and `/SetSpatial` with the exact endpoint ID and format GUID. The original selected/active format is currently Dolby `{<identifier-removed>}`; fresh getters must confirm it. NirSoft's documented syntax is a rehearsal, not a substitute for that fresh snapshot.

Expected original runtime is 189,308 bytes, SHA-256 `<private-artifact-digest>`; device data is 248 bytes, SHA-256 `<private-artifact-digest>`. Retained personalized Dolby API is 62,791 bytes, SHA-256 `<private-artifact-digest>`; registry is 62,799 bytes, SHA-256 `<private-artifact-digest>`. Drift from those payloads ends preflight. The old unpersonalized Dolby payload is not a restoration target.

Capture original DTS twice, converted DTS twice, then restored original DTS twice using fresh experiment-owned streams. Inputs are already generated and tested inprepared_capture_20261014: six cardinal positions, two held-out elevated positions, repeated front/left; radius1m; one dynamic object; `AudioCategory_SoundEffects`;48 kHz mono float32;1s exponential sweep20Hz–20kHz at−34dBFS,250ms pre-silence and2s tail. Each run is approximately33s. Capture stereo float32/48 kHz from the paired virtual endpoint without changing its negotiated format. Preserve complete raw recordings, packet timestamps and render timestamps.

Compare unnormalized responses with one common stereo latency adjustment. Measure direction-dependent spectra, impulse peaks, ITD and relative ear levels, repeatability and restored/original agreement. At exact front/side nodes, compare the candidate capture against the loaded FIR pairs after accounting for unchanged native post-processing. Differences alone are not enough: changed response must follow the candidate and reverse on restoration. Report held-out directional discrepancies separately from float32 coefficient error. The converted field is an approximation on22 directional nodes, not a lossless710-direction installation.

## Execution bounds and restoration

Usedts_profile_control.ps1 Backup/Validate before mutation. Apply/Restore require the approved-test flag. Each setter worker runs in an owned hidden process with a15s deadline; normal source/capture deadlines are60s per run. Native0x180007948 can wait on its DSP event, so a blocked write is not retried with a service restart, provider cycling or additional field edits. If a worker times out, stop only its owned process, read actual values, then invoke the original BLOB/counter restoration in a separate bounded worker if values still match the test's original/candidate states.

An outer cleanup always restores changed DTS properties and then the prior spatial format. Verify both via fresh independent getters and compare the complete saved unrelated state. If Windows automatically changes a persisted format during provider selection, stop recording and restore the prior provider; additional format writes are not included in this plan. If exact restoration fails, retain the fresh backups and precise failed action; do not improvise other changes.

The separate capture engine SHA-256 is `<private-artifact-digest>`. Its build, getter-only execution and refusal paths passed. The new native recording route and real writable-store behavior have not run because they are part of this checkpoint. This approval would cover the listed temporary provider selection, two DTS properties, six bounded virtual captures and exact restoration only.
