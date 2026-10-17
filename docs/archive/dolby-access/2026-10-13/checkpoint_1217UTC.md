> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Dolby Access PHRTF checkpoint — 2026-10-13 12:17:49 UTC

This checkpoint records verified evidence since 11:55 UTC. It supersedes earlier dated status statements where they conflict. The investigation remains static/offline; no candidate has been imported.

## Verified evidence

- The read-only endpoint inventory at `audio_endpoints_readonly_20261013.json` (11:56 UTC) reports selected virtual endpoint as console and multimedia playback default and selected virtual endpoint as the paired recording endpoint. Both persisted formats remain stereo 48 kHz PCM24. The read-only WinRT query `spatial_format_readonly_20261013.json` (11:59 UTC) reports Dolby Headphones as the active/default spatial format. PCM16 is an optional stimulus choice, not an endpoint requirement.
- Static copied-DSP evidence maps property `{1b4dab55-b1fb-4d8c-8317-f2d4a96efbb8},1` to `PKEY_DAPVR_INITTIME_DAHP`, with Dolby context `{45da5c30-2837-4ac4-b1e2-50acc3865974}`. The normal DAHP initialization branch selects it; a conditional CAPX alternate selects `{<identifier-removed>},1`. `ApplyModuleParams` reads a blob and follows `SetParams` -> `SetInitTimeParams` -> `ConfigureInitTime` -> `DecodeMessageTuning`. Evidence: `dsp_property_ownership_20261013.json`; static evidence IDs include `ev_281f70818552f9b69627a14b416884ad6b434d2c25615e4bf2e8a2087f961482` and `ev_97eb74d3312e8ad25c711cebedfffa7802361fc4a882c64b26951ec7e531f572`.
- Read-only property-store evidence `effect_property_readonly_20261013.json` (12:08:39 UTC) records STGM_READ and successful activation/open/getters. DAHP is a 14-byte VT_BLOB, SHA-256 `<private-artifact-digest>`; byte comparison confirms it equals the saved 22-byte registry value after stripping the 8-byte envelope. CAPX reads VT_EMPTY. No setter or commit ran.
- `native_import_verification_20261013.json` reports all seven verification groups passed: copied-PE property/context GUID identity, exact endpoint/context/property, owned source identity, API/registry envelope split, tuning-v2 array/version equality, preservation of all other init fields, and equality to the saved read-only API baseline. It explicitly does not prove native invocation, runtime mode, acceptance, output, or persistence.
- The private owned source is 201,590 bytes with SHA-256 `<private-artifact-digest>`. Candidate artifacts remain private under `private/native_import_candidate_20261013/`: registry value 62,799 bytes, SHA-256 `<private-artifact-digest>`; API payload 62,791 bytes, SHA-256 `<private-artifact-digest>`. Baseline registry/API values are 22/14 bytes. The generator hard-checks the unique endpoint property, expected baseline and owned-profile hashes, and writes separate API candidate/baseline payloads.
- The selected extended profile checksum path with an 11-word trailer matches; the core-only XOR check fails. This is a ported static check, not native DLL acceptance. Five tests, candidate generation and verification, and Python compilation passed after adding postponed annotation evaluation for Python 3.8 compatibility. `scripts/read_effect_property.ps1` completed with zero PowerShell AST parse errors.

## Current operation and next step

The REA DSP session is open. The primary agent is reviewing six Ghidra cache-model/init targets; target mapping succeeded without analysis. Trace: `ghidra_cache_continuation_20261013.txt`. Exact cache JSON/digest encoding remains unresolved. Next, continue that focused static review and export updated REA evidence after the final SetParams trace.

## Blockers and unsuccessful paths

- The conditional CAPX branch exists, but current CAPX property state is VT_EMPTY and runtime branch selection is unknown. Native tuning acceptance is untested; SetParams success can conceal decode fallback.
- LocalCache outer JSON/MD5 serialization and cache persistence remain unresolved. The proposed candidate test is documented at `reports/2026-10-13/native_import_test_plan.md`; it awaits <identity-removed>'s direct approval for the exact endpoint property. The later audio probe is separate, unbuilt, and not execution-ready pending review of stimulus and stream details.
- The current process token is not an administrator. No writable-store permission probe was attempted. If the approved property operation is denied, stop and use the authorized helper path; do not elevate or bypass access controls.
- Earlier attempts to relay updates to Dot failed with `AbsolutePathBuf` deserialization errors. Do not repeat the unchanged relay call.
- Python 3.8 initially rejected PEP 604 annotation evaluation in the generator; postponed annotations fixed it and the test/generation/verification/compile sequence then passed. Core-only XOR was the wrong checksum hypothesis; only the extended 11-word trailer check matches.

## Live changes and rollback

No live Dolby property setter, Commit, registry write/import, profile-file mutation, Dolby Access state change, app launch, stream activation, audio playback, or capture occurred. Read-only property/endpoint queries and private offline artifact generation are the only activity. No rollback was needed. Existing backups are reference evidence, not a substitute for fresh independently checked backups immediately before any separately approved test.

## Exact paths

- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`

## Timestamped session-closure update — 2026-10-13 12:20:39 UTC

The REA DSP session was exported and successfully closed. Final export: `<local-path>` (34,512,625 bytes). Snapshot: `<local-path>` (68,918,255 bytes). Final static `SetParams` and `SetInitTimeParams` pseudocode is in `<local-path>`; it does not establish native tuning acceptance.

Current operation advanced to offline .NET Native import-resolution for the MD5 Encoding accessor. The inert copy `<local-path>` is 13,418,880 bytes, SHA-256 `<private-artifact-digest>`. The installed manifest requires Microsoft.NET.Native.Framework 2.2. This is only a dependency lead; no new encoding result is verified. Next step is offline accessor resolution without invoking the DLL.

The investigation remains offline static analysis plus read-only getter observations. No property setter, Commit, registry write/import, profile or app-state change, audio stream, playback, or capture occurred. No rollback was needed; any approved property test still requires fresh independently verified backups. The exact-property proposal remains pending direct <identity-removed> approval, and the later audio probe remains separate and unbuilt.
