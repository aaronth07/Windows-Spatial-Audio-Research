> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Checkpoint log — 2026-10-13

Append dated entries as verified updates arrive. Keep prior-session observations labeled as such.

## Initial checkpoint

- Scope: static/offline investigation of a legitimate owned PHRTF import into consumer Dolby Access and native object-based Windows Spatial Sound playback.
- `<local-path>` readable; length 43,118 bytes; SHA-256 `<private-artifact-digest>`.
- REA MCP 6.4.0 active; Ghidra 12.1.4 available.
- Archive inspection evidence: `ev_cf068afd9624e09324bb57dcaeb179a9d4632f30fa49510f70511d02f9eb22b8`.
- Inventory evidence: `ev_8c0c8c23ab5b298155c33ad4c04ca1ae0b6ceea93c8ee3b1998ac703261f96f1`.
- Handoff documents read. Manifest, scripts, and tests remain unvalidated and unexecuted.
- Prior-session `STATUS.md` and `CHECKPOINT_EVIDENCE.md` reviewed as historical context; their static observations were not re-run for this entry.
- No live application, audio, account, device, or system change; no activation or playback proof.
- Coefficient findings remain in the separate deferred record.

## Follow-up checkpoint

- `inputs/HANDOFF.zip` CRC passed; all 27 manifest hashes verified across 28 members. Archived `REPORT.md` matches the attached report byte-for-byte (SHA-256 `<private-artifact-digest>`).
- Extracted archive under `inputs/handoff_20261013/Dolby_PHRTF_HANDOFF`; retained copies `inputs/HANDOFF.zip` and `inputs/Dolby_PHRTF_Research_Report.md`. Audit record: `handoff_audit_20261013.json`; corrected `scripts/audit_handoff.py` rerun succeeded after the initial plain-text-header parsing error.
- All four handoff synthetic Python tests passed. Collector PowerShell AST parse returned zero errors; collector execution remains pending.
- Attached owned profile SHA-256 `<private-artifact-digest>` matches the existing private copy; this proves byte identity only.
- Inert consumer `x64/DolbyHrtfEnc.dll` static inspection found object encoder methods/strings. Method VA `0x180007b6c` handles object type, position, and gain; initialization VA `0x180005c40`. REA evidence `ev_474e6ebf3b18bbd51d4f8823a8c73298611e28659b7eb63a03bdca308173a761`; saved records in `private/rea_hrtfenc_20261013.json` and `private/hrtfenc_20261013.rea-snapshot.json`. No PHRTF consumption is proven.
- `DAX.winmd` and `Dolby.SpatialCodecs.winmd` evidence and consumer Atmos manifest identifiers are recorded in the dated report. Access remains 3.27.12250.0; Atmos for Headphones remains 2.3.303.0. No app launch.
- Next static target: Access serialization/property schema and background IPC coefficient transfer. No live app/system changes or playback proof.

## Settings authorization and static tuning-object checkpoint

- <identity-removed> authorized project-related Dolby Access settings changes and use of already-open elevated Notepad++/PowerShell through computer control if ordinary commands lack privilege. Any test is bounded by an explicit PHRTF plan and verified backup/rollback. No approved profile-file mutation or test plan exists yet.
- Reported intended state to preserve: default `Stereo Convolver`; Dolby Atmos for Headphones, 2-channel/48 kHz/intended 16-bit; Dolby Access Game mode Performance, Intelligent EQ off. Actual settings still require read-only verification.
- Boundaries: no default-output/routing/Vivaldi changes; no KS Repeater or bridges; no physical-speaker output; no driver, service, binary, license, account, or pro Renderer changes. Output tests await loopback-only confirmation; no playback. Native computer APIs are disabled in the available namespace; elevated UI capability is unknown.
- `ghidra_audio_path_trace_20261013.txt`: non-OEM `PhrtfAdjustment` VA `0x183441be0` serializes `this+0x18` with `FUN_1832b2350` format 1, deserializes/casts to concrete type pointer `0x180071828`, and assigns at configuration `+0x20`. OEM VA `0x183441700` performs analogous assignment after endpoint guard. Exact schema and DSP use remain unknown.
- Candidate map `access_continuation_map.json` (from `scripts/map_access_continuation.py`) has 288 UTF-16 strings, 552 RIP references, and 215 pdata functions. Nine focused functions were decompiled with `AccessStatic`; no global analysis. These are candidate inventories.
- DAX RPC property setter VA `0x1800078d0` forwards strings to a native RPC wrapper; evidence `ev_578bc76619e975e32a8ea0c02728db4a84d3fffd816c7de0356052997af54662`, records `private/rea_dax_20261013.json` and `private/dax_20261013.rea-snapshot.json`. No PHRTF property key verified. Background-update IPC remains primary next trace.

## Administrator authority correction and DSP decode checkpoint

- Permission correction: this research agent has no administrator authority and must never use elevated apps directly. Admin-locked targets go to dot with exact target, purpose, and requested read/write for dot's PC helper. Earlier relayed elevated-app authorization is superseded.
- Non-admin Dolby Access setting authorization and endpoint/output restrictions remain. Registry backup path is unknown and reference-only. No live registry write/import without separate <identity-removed> approval for the exact key/value; never import a full registry backup.
- Inert `DLLs/x64/DolbyAudioProcessing.dll`, SHA-256 `0fada40251880d03c657a357745d362799c93fb6024f733f1c8bf1057f5e4715`, analyzed via REA/Ghidra 12.1.4. `DecodeMessageTuning` VA `0x18011c450`, evidence `ev_e7dec6887f6749de4750170b4a4b6978615a191bb1b7a6ef218741ea6f89691d`: priority v3/v2/v1; v1/v2 reduce 32-bit coefficient values to low16 then little-endian serialization/native deserialization; v3 decodes quoted base64 blob-list text. Both native room and PCA coefficient pointers must be non-null for success.
- `ConfigureInitTime` VA `0x180123260`, evidence `ev_d181960db38ded1e5de840c6aedb81a80cb6b52ec0a0c19dc905d33cc3f24962`, consumes an init blob through `0x180122950`. Raw evidence remains in active REA, export to `private/rea_dsp_20261013.json` pending. No owned profile invocation/load.
- Main Access manifest registers `Dolby.DolbyAtmosForHeadphonesEncoder` at `DLLs/x64/DolbyHrtfEnc.dll` and `Dolby.DolbyAudioProcessingModule` at `DLLs/x64/DolbyAudioProcessing.dll`; this is the active implementation lead. The separate package app-service declaration remains package metadata.
- Next: inspect container/deserialization routines `0x1801082c0` and `0x18010cb30`, main Access init serializer fields, and background-update IPC. No playback or app launch.

## Final checkpoint and idle state

- Registry reference `<local-path>` was examined offline and copied byte-identically to `private/codex_reference-10-13-26.reg`: 22,750 bytes, UTF-16LE BOM, SHA-256 `<private-artifact-digest>`. It contains only `HKCU\Software\Microsoft\Windows\CurrentVersion\GameDVR` and its `Debug` subkey. No Dolby/PHRTF/audio endpoint keys. Sanitized record `registry_reference_20261013.json`; debug value data omitted. No live registry import/write and no patch justified.
- Admin correction: no admin authority; never use elevated apps directly. Ordinary commands are allowed only when existing privilege/tools support the authorized operation. Admin-locked target goes to dot with exact target, purpose, requested read/write; dot's PC helper is fallback. Exact key/value registry approval remains separately required; never import full registry backup.
- REA native session closed after export: `private/rea_dsp_20261013.json` (15 records, 14,763,461 bytes); snapshot `private/dsp_20261013.rea-snapshot.json` (29,486,868 bytes).
- DSP static dossiers: memory sizing at `0x1801082c0`, evidence `ev_6051ae350697009c512d7b9f62e340165ba4dc40f9c098f631e425cba2be39cc`; deserializer at `0x18010cb30`, evidence `ev_01a746899b2e5c24295fdc47e6ae0bae4210b21d63d71a6bafaedf270608939c`. Both follow wrapper `0x18010bf10`; exact checksum remains untraced.
- `0x180122950` (`ConfigureDapVRInitParams`), evidence `ev_d47949047bdd27f5608a28024013860b2e92ad83a5231f41a4de2bd2144bfa4b`, parses init blob through `0x180150220`, then calls `DecodeMessageTuning`; success stores room/PCA pointers in init config.
- `ghidra_audio_followup_20261013.txt`: wrapper `0x18386b1e0` calls prior `UpdateParameters`; profile-cache body `0x183839430` retains unresolved indirect calls. `0x18386b8b0` was renamed `function_endpoint_property`; it is not verified serialization. `_setTestParameters` is a debug/request handler, not established production import. Guessed `0x183886c00` yielded no established function and is excluded.
- Handoff audit rerun verified the existing extraction without overwrite; both new scripts passed `py_compile`; continuation mapper rerun reproduced 288 strings, 552 refs, 215 functions. Original registry/profile/inputs unchanged. No live app/audio/registry mutation.
- Idle awaiting dot. Next continuation: type/schema serialization into native init blob, base64 wrapping, exact container checksum, background-update IPC, read-only endpoint verification, then reviewed test and backup/restore plan. No import/playback established.

## Correction: static work active

- The latest instruction supersedes the idle request. Continue static work; REA remains closed with evidence saved until the next targeted opening.
- Already-authorized backed-up operations may be performed directly when actual access permits; dot/Luna helper is fallback if access is unavailable. Do not assume access or bypass denials.
- Primary next work: resolve exact LocalState versus LocalCache storage using WinRT interface GUIDs/vtable slots, then initialization tuning schema/native payload serialization. Prepare exact backed-up tests within the authorized scope. Separate exact live registry approval and loopback-only output confirmation remain pending.

## Store root resolution and capture authorization

- Generic store constructor `0x182581660` resolves through `ApplicationData.Current` (slot 6), `LocalCacheFolder` (slot 6), and `IStorageItem.Path` (slot 12). WinRT IIDs: `<identifier-removed>` (`IApplicationDataStatics`), `<identifier-removed>` (`IApplicationData2`), `<identifier-removed>` (`IStorageItem`). SDK method order and `param3*8` wrapper calls confirm slots; traces `ghidra_storage_root_20261013.txt` and `ghidra_storage_root_call_20261013.txt`.
- Derived package LocalCache filenames: `phrtf.json` and `phrtf_{id}.json` under `<local-path>`. This does not establish file presence or loading. Microsoft documents LocalCache is excluded from platform backup/restore; a test requires a separately verified file-level backup.
- Capture authorization: paired Stereo Convolver RECORDING endpoint via WASAPI and current default selected virtual endpoint PLAYBACK endpoint for stimuli. Loopback-only is a fallback only if available. Read-only verify intended 16-bit stimuli and no microphone capture; <identity-removed>'s Dolby-processed tap confirmation is user verification. No routing/monitoring changes; final capture follows candidate import; no playback yet.
- Continue with initialization tuning schema/native payload serialization and the read-only MMDevice endpoint/format inventory before the exact test proposal.

## Full registry, endpoint, and parser checkpoint

- Direct registry handoff replaced the earlier small export as current reference: `private/registry_reference_full_20261013_01e41966ecba.reg`, 930,053,518 bytes, SHA-256 `<private-artifact-digest>`, 832,713 keys. Independently rehashed copy verified. Root index `registry_reference_full_index_20261013.json` is counts/hash only; full index and endpoint excerpts are private. Earlier GameDVR-only version remains private.
- Name-only search for `phrtf`, `rosella`, and `headphone_tuning` had zero literal hits; binary values are not covered, so no absence claim. Eight offline endpoint REG_BINARY blobs are indexed by length/hash in private metadata. Do not identify FX entries `45da` or `08e54` without GUID mapping. No live registry mutation.
- Read-only endpoint inventory: console/multimedia default selected virtual endpoint render ID `{<identifier-removed>}`; paired active recording selected virtual endpoint `{<identifier-removed>}`. Both stereo 48 kHz PCM24 valid24; intended 16-bit is not the endpoint mix format. No format change. Communications default is separate and unused. Initial helper failed due to wrong collection IID and name filter; both fixed from SDK and successful rerun used no audio activation.
- <identity-removed> verified the paired recording endpoint captures Dolby processing; this is user verification, not an independent measurement. Capture is permitted via paired WASAPI record endpoint with current default playback stimuli; loopback fallback only if available. Read-only verify stimulus depth, no microphone, and processed tap. Final capture validation is after candidate import; no extensive harness or premature capture.
- Current LocalCache was checked and contains no PHRTF files. `settings.json` last modified May 9 is stale for current UI; current log keyword summary is counts only. This is bounded evidence, not absence proof for other state.
- Parser `0x180150220` evidence `ev_3d659ec814cc6951840dd0b92fa606fc3bae5993ea902d755cd8f249d311f33f` reads optional-presence booleans, uint32, integer arrays, bounded version, and base64 strings; not protobuf. `0x1801509f0` evidence `ev_bdc32ccd7a6a4a6011190bc739bc54dd88b8799c6130662190b63370999b6e03` caps arrays at 32768 and version at 20 bytes. `0x18010bf10` evidence `ev_42b60c07ea4bd0cf0c47a23b8e8a371299cf68a0e26326b1bcc5f5a54079f790` uses tag/word XOR integrity with seed `0xa569` and checksum tag `0x8042`. Fast path `0x18010dab0` evidence `ev_4c273a6263859b08634c30a20b662db3f6248f948e0e069c1030e77a9bc841c5` recognizes magic `0x7072` and validates independently; exact port incomplete.
- Next: main initialization tuning schema/native payload conversion and exact container port; after candidate import, complete read-only endpoint-format and Dolby-tap validation. No candidate import or playback.

## Native serializer bridge and read-only live state

- Access init serializer `0x183636b10` has a seven-field nullable layout matching native reader `0x180150220`/`0x1801509f0`. `PhrtfAdjustment` writes configuration `+0x20`; child field 8 is tuning v2 via `0x183634f50`. Writer `0x1836367d0`: v1 count + signed int32 array; v2 adds version length/raw bytes; v3 uses two uint32 lengths, base64 blob-list bytes, then version bytes. Decode order v3 > v2 > v1. See `ghidra_tuning_serialization_20261013.txt`, `ghidra_tuning_v1_writer_20261013.txt`.
- Offline candidate [`native_import_candidate_20261013.json`] plus private payloads: v2, 62,799 bytes, SHA-256 `<private-artifact-digest>`; baseline value 22 bytes, SHA-256 `<private-artifact-digest>`. It preserves all three bools, int 0, absence of v1/v3, and inserts 15,691 signed int32 values with existing 5-byte version. Five canonical round-trip/bounds/truncation/malformed tests pass. No import/write.
- Fast-path offline check: core XOR fails; extended XOR matches. Words after header: 15,678 core + 11 trailer = 15,689, plus two header words = 15,691. XOR low16 words `[2:15691]`, seed `0xa569`, checksum in word 1, magic word 0 excluded. Estimated memory 67,032 bytes under `0x21380`; selected checks do not establish DLL acceptance. Exact port and separate outer settings MD5/typed-JSON remain unresolved.
- Read-only endpoint registry value at `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\MMDevices\Audio\Render\{<identifier-removed>}\FxProperties\{45da5c30-2837-4ac4-b1e2-50acc3865974}\{1b4dab55-b1fb-4d8c-8317-f2d4a96efbb8},1` matched the exported 22-byte baseline at `11:52:05Z`; evidence `live_registry_baseline_read_20261013.json`, private fresh backup `live_read_backup_20261013_065205.bin`. Property ownership/GUID mapping unresolved; do not call candidate accepted Dolby state. No write approval.
- Correct WinRT query at `11:59Z`: `SUPPORTED=true`, active/default Dolby Headphones format GUID `<identifier-removed>`; `spatial_format_readonly_20261013.json`. Earlier unsupported raw-ID probe is excluded as invalid state evidence. No setter, stream, playback, or capture. Mix format remains stereo 48 kHz PCM24.
- LocalCache currently has no PHRTF files; settings.json is dated May 9 and stale for UI state. Log keyword summary is counts-only. Capture tap Dolby processing is <identity-removed>-verified, not independently measured. Candidate import remains pending separate exact registry write approval and primary test/restore proposal; final capture is only after candidate import.

## Follow-up: init wire bridge and current read-only state

- Offline Ghidra tracing closes the Access init serializer to native DSP reader: serializer `0x183636b10` has seven nullable fields matching `0x180150220`/`0x1801509f0`. `PhrtfAdjustment` writes configuration `+0x20`; the init writer calls `0x183634f50` on child field 8 (tuning v2). Writer `0x1836367d0` emits v1 count + signed int32 array, v2 plus version length/raw bytes, and v3 as two uint32 lengths plus base64 blob-list bytes and version bytes. Native decoder `0x18011c450` prefers v3, then v2, then v1. Source traces: `ghidra_tuning_serialization_20261013.txt`, `ghidra_tuning_v1_writer_20261013.txt`, and earlier profile/types and audio traces. This is a static wire-layout bridge, not proof of import or live acceptance.
- Offline candidate remains a 62,799-byte v2 value, SHA-256 `<private-artifact-digest>`; baseline is 22 bytes, SHA-256 `<private-artifact-digest>`. Five canonical encoding, bounds, truncation, and malformed-input tests pass. Selected fast-path/XOR and estimated-memory checks are documented in the report; these do not prove DLL acceptance. Candidate files remain private and unimported.
- Current live endpoint registry value was read at `2026-10-13T11:52:05Z` and byte-matches the exported 22-byte baseline; evidence `live_registry_baseline_read_20261013.json`, private read backup `private/live_read_backup_20261013_065205.bin`. FX property ownership is unresolved, and exact registry write approval is absent. No registry mutation.
- Corrected read-only WinRT query at `2026-10-13T11:59Z` reports `SUPPORTED=true` and active/default Dolby Headphones format GUID `<identifier-removed>`; endpoint mix is still stereo 48 kHz PCM24. No setter, stream activation, playback, or capture. Paired capture tap is user-verified, not independently measured.
- Read-only endpoint inventory remains selected virtual endpoint playback default and selected virtual endpoint paired recording endpoint, both stereo 48 kHz PCM24. Intended 16-bit stimulus path and absence of microphone capture require read-only verification before any exact test. Final capture remains gated on candidate-import approval. Dolby Access Game mode/Intelligent EQ has not been read from the current UI.
- Current next work: focused FX property ownership metadata and primary-agent exact test/backup/restore proposal. Separate exact registry write approval is still required; LocalCache outer MD5/typed-JSON persistence, native container exact port, UI acceptance, restart persistence, import, and playback remain unresolved. Static investigation active; REA closed with saved evidence pending the next targeted opening.

## Historical current-state correction — superseded by the 13:13 UTC update

- Supersedes the immediately preceding checkpoint's status wording: the REA DSP session is open after reopening its saved snapshot. The selected native container checks are now ported; the owned 201,590-byte profile's SHA-256 matches its supplied identity, and size/checksum checks pass using the extended 11-word trailer path. The core-only XOR path fails. These results do not establish native DLL acceptance. Candidate manifest `native_import_candidate_20261013.json` records 15,691 words, 15,678 core words, 15,689 extended words after the header, `extended_xor_matches=true`, and its stated limitation that this is not native execution/acceptance.
- PCM16 is optional as a stimulus choice, not a required endpoint format. The read-only MMDevice record verifies stereo 48 kHz PCM24 persisted formats; no endpoint format change occurred.
- No live registry write, candidate import, app launch, stream activation, playback, or capture occurred. Exact registry key/value approval remains absent; native acceptance and output proof remain pending. The primary agent is preparing the exact test and backup/restore proposal.

## Earlier DSP property mapping and read-only API check — branch interpretation corrected at 13:13 UTC

- Static consumer DSP evidence maps `{1b4dab55-b1fb-4d8c-8317-f2d4a96efbb8},1` to `PKEY_DAPVR_INITTIME_DAHP` at `0x18012b5e0` (evidence `ev_281f70818552f9b69627a14b416884ad6b434d2c25615e4bf2e8a2087f961482`). `SetAPOInitParameters` at `0x18012c450` uses the Dolby context GUID `{45da5c30-2837-4ac4-b1e2-50acc3865974}`. Normal DAHP init selects the mapped key; a conditional CAPX path selects `{<identifier-removed>},1`. The machine-readable record is `dsp_property_ownership_20261013.json`.
- Static parameter flow: `ApplyModuleParams` reads `VT_BLOB` and calls `SetParams` (`0x180124d00`) -> `SetInitTimeParams` (`0x180124f50`) -> `ConfigureInitTime` (`0x180123260`) -> `DecodeMessageTuning`. `HandleNotification` matches Dolby context/store/type before dispatching init. The CAPX/DAHP runtime branch is unverified; `SetParams` may report success despite decode fallback, so return status alone is not acceptance evidence.
- Read-only property-store check `effect_property_readonly_20261013.json`, observed `2026-10-13T12:08:39Z`: STGM_READ activation, open, and getters returned S_OK. The DAHP property read as a 14-byte VT_BLOB, SHA-256 `<private-artifact-digest>`; directly compared bytes match the 22-byte registry baseline after stripping its 8-byte envelope. CAPX alternate is VT_EMPTY. No setter, commit, stream, playback, capture, or registry write.
- Test/backup proposal is in `reports/2026-10-13/native_import_test_plan.md`; exact property approval is pending <identity-removed>. The later audio probe still needs remaining stimulus and stream details reviewed before execution. No live operation is authorized by the proposal itself.

## Current checkpoint — evidence through 2026-10-13 13:13 UTC

This entry supersedes prior current-state wording in the chronological log where it conflicts.

- At 2026-10-13T13:06:30.6596299Z, after <identity-removed> closed Dolby Access and manually turned Atmos off, the read-only backup manifest recorded backup_complete=true, Access process count 0, and active/default spatial GUIDs {00000000-0000-0000-0000-000000000000}. The snapshot performed no spatial enablement or property write. Fresh property registry/API values, six User-context values/export, and app-file copies are in native_property_test_backup_20261013.json. No rollback was needed.
- The exact property set/restore remains approved but paused pending Dot's evidence review. <identity-removed> designated A prior reviewer (local thread <identifier-removed>) as future coordination route; the older durable relay failed. No questions should be routed to <identity-removed> while he is AFK.
- The 13:13:20 read-only hive audit used unchanged working duplicates. Dolby Access parsed as 5 keys/49 values; Dolby Atmos parsed as 3 keys/0 values. Both had valid hive structure, no decode errors, and unchanged source/duplicate pairs. Stored Access Game profile values are Performance true, Intelligent EQ Off, Volume Leveler false; these are archived hive values, not current UI state. HasSyncedProfile is JSON string "true" and does not prove PHRTF synchronization. Auto-enablement entries carry timestamps, not proof of current activation. No explicit PHRTF name/data appeared in scanned decoded text; this is bounded negative evidence. No hive edit is justified. See settings_hive_inspection_20261013.json.
- Static branch correction: alternate DAFM/Atmos for Monitors init property {<identifier-removed>},1 is selected by the this+0x738 presence/status check, not CAPX availability. The normal User-property path uses the system-effects/CAPX framework. Native runtime was not observed; see dsp_branch_selection_20261013.json.
- Owned-profile word count is 15,691 including two outer header words, 15,689 after the header (15,678 core + 11 trailer). The extended trailer path matches; core-only XOR fails. Digest/serialization evidence confirms US-ASCII code page 20127, replacement fallback ?, MD5 uppercase X2, and formatter value 1 with CRLF writer defaults; exact JSON schema/order remains unresolved. See access_digest_encoding_verified_20261013.json.
- No app-hive edits, property setter/commit, candidate import, playback, or capture occurred. Earlier 11:59 spatial-active query and 12:45 incomplete backup are historical and superseded by the post-user-action 13:06 observation.

## Checkpoint — 2026-10-13 13:28 UTC; property round trip passed and restored

This entry supersedes the 13:13 current-state checkpoint and earlier status wording where they conflict.

A prior reviewer reviewed the exact previously approved property-only test. The immediate fresh backup at 13:27:59.6392372Z records backup_complete=true, Access process count 0, and active/default spatial GUIDs zero. The one exact property was set, committed, and read back, then restored and verified to its original baseline; all API calls returned S_OK. See property_round_trip_20261013.json and private/native_property_test_20261013T132759546Z/backup_manifest.json.

The selected endpoint was {<identifier-removed>}, context 45da5c30-2837-4ac4-b1e2-50acc3865974, property {1b4dab55-b1fb-4d8c-8317-f2d4a96efbb8},1. Candidate API payload: 62,791 bytes, SHA-256 <private-artifact-digest>; registry representation: 62,799 bytes, SHA-256 <private-artifact-digest>. Restored API baseline: 14 bytes, SHA-256 <private-artifact-digest>; restored registry representation: 22 bytes, SHA-256 <private-artifact-digest>.

Post-restore verification confirmed six User-context values, app files, defaults, and Atmos-off state. No audio stream, playback, capture, spatial command, Dolby Access launch, or additional live operation occurred. This establishes writable property persistence and immediate restoration only; native PHRTF loading, personalized processing, Access integration, and restart persistence remain unproven.

The 13:22 dependency_rollback_assessment_1322UTC.md said review was pending when written; that wording is historical and superseded by Prior review and this result. Exact authorization for other registry values remains separate. Static dependency evidence is now exported and REA is closed: private/rea_dsp_dependencies_20261013.json and private/dsp_dependencies_20261013.rea-snapshot.json.


## Checkpoint — 2026-10-13 14:20 UTC; converter rule resolved, runtime still unreleased

This entry supersedes the 13:55 current-status summary where it conflicts.

The 13:27 property candidate readback and immediate baseline restoration remain verified; no further setting change occurred. The later property verification records unchanged endpoint inventory and Atmos-off state. Current persisted format is selected virtual endpoint playback, stereo 48 kHz PCM16, with paired selected virtual endpoint recording, stereo 48 kHz PCM24. Earlier playback PCM24 is historical.

The matching-PDB-verified date converter uses constructor 0x18274bc30, type 0x18021aff8, and callback 0x183661090. Style 0x10 converts supported DateTime/DateTimeOffset values to UTC before InvariantCulture formatting with yyyy'-'MM'-'dd'T'HH':'mm':'ss.FFFFFFFK. This converter behavior is resolved, but which PHRTF properties use each date type remains unknown. Callback 0x18274be70 is general contract creation, not a proven property filter. Exact field associations/order, Guid text serialization, and the complete cache activation/configuration contract remain unresolved; InstallationId is not directly parseable as a Guid and is not substituted.

Native C++/wrapper/comparison preparation compiled and passed offline checks. The runner's corrected SHA-256 is <private-artifact-digest>; the other four pinned hashes are unchanged. Offline tests include 20-second owned-process and 15-second spatial BAT deadlines, success/exit-1 handling, termination/disposal, and pre-COM null-callback rejection. No native runtime, stream, or spatial BAT ran. Dot review of five hashes and the six-capture sequence remains pending; desktop is clear for EQ.

Windows 11 Pro build 26200 is observed. SoundVolumeView 2.36 compatibility is unverified. No registered Event Log provider matched *Dolby*; private ETW remains outside that bounded query. Evidence files are native_load_preparation_20261013.json, property_round_trip_verification_20261013.json, access_date_converter_20261013.json, cache_schema_links_20261013.json, existing_dolby_log_inventory_20261013.json, and reports/2026-10-13/cache_serialization_addendum_1408UTC.md. The 13:55 proposal is unchanged.

## Checkpoint — 2026-10-13 14:43 UTC; mocked default-guard tests passed

This entry supersedes the 14:20 current-status summary where it conflicts.

Dot's exact-default correction added fresh console and multimedia endpoint checks in initial preflight and immediately before each of the two selected spatial BAT launches. Wrong, missing, duplicate, or unreadable defaults block launch with phase and observed IDs recorded. Wrong-default cleanup records cleanup_incomplete. The C# fixed-property restore remains before default verification, and outer fixed-context/app-file backup verification is independent of the OFF guard.

Fourteen mocked cases passed: ten blocked BAT cases, two allowed mock cases, an initial preflight guard, and cleanup recording with fixed-backup verification still reached. No real MMDevice query, BAT, property API, or audio stream was invoked. Do not describe this as actual BAT execution or real-stream restoration testing.

The corrected PowerShell runner SHA-256 is <private-artifact-digest>; the other four pinned artifacts and BAT hashes are unchanged. PowerShell 7.6.5/.NET 10.0.11 host checks passed, including owned parent/child termination with a separate witness preserved, and C# layout/hash/JSON checks. Native runtime remains unreleased. Dot review of the comparison and plan remains pending.

Evidence: native_default_guard_1440UTC.md, native_default_guard_1440UTC.diff, native_default_guard_tests_20261013.json, native_host_compatibility_20261013.json, native_load_preparation_20261013.json, and native_load_validation_plan_1355UTC.md. No proposal, source, or evidence artifact was edited for this reporting entry.

## Checkpoint — 2026-10-13 15:10 UTC; candidate-dependent native output verified and restored

This entry supersedes earlier pending-release, no-stream, and no-native-load statements where they conflict. It preserves those entries as historical records of what was known at their creation.

The reviewed six-run sequence completed from 15:02:34.1055429 to 15:03:55.7500375 UTC with success and exit code 0. The approved property candidate was set, committed, read back, then restored and verified; all set/commit HRESULTs were S_OK. Candidate API/registry readbacks were 62,791/62,799 bytes and matched SHA-256 `<private-artifact-digest>` / `<private-artifact-digest>`. Final baseline API/registry readbacks were 14/22 bytes and matched `<private-artifact-digest>` / `<private-artifact-digest>`.

Six current registry User values and app files independently matched the fresh backup. Endpoint inventory and both console/multimedia defaults remained identical to the approved state. Atmos was off, active/default spatial GUIDs were zero at 15:03:54.9919007 UTC, and `cleanup_incomplete=false`. No further live changes or retests occurred.

Six native stream/capture runs returned S_OK at all recorded stages, using mono float32 48 kHz stimuli with one dynamic object and paired stereo float32 48 kHz recording. Persisted endpoint formats remained playback stereo 48 kHz PCM16 and recording stereo 48 kHz PCM24. Relative candidate differences at front/left/right/rear/above were 0.244751/0.392137/0.384137/0.232274/0.340573. Baseline and restored control variation was zero, as was candidate-repeat error after a shared stereo lag (candidate1 −122 samples, candidate2 0). Whole-capture comparison over 502,740 aligned frames confirms exact control equality and candidate-repeat equality, and that candidate output differs.

The repeatable candidate-dependent capture supports changed native processing from the PHRTF property in this test. It does not directly prove full internal coefficient decoding or room/PCA pointer use; those remain static-code inferences. Access UI/cache integration, restart persistence, persistent enablement, and acoustic/perceptual validity were not established. The fivefold-control threshold is an operational discriminator rather than a statistical probability.

Copied report evidence: `native_load_validation_manifest_20261013T150234.json` (7,837 bytes; SHA-256 `<private-artifact-digest>`), `native_load_capture_comparison_20261013T150234.json` (4,730 bytes; SHA-256 `<private-artifact-digest>`), and `native_load_verification_20261013.json` (21,803 bytes; SHA-256 `<private-artifact-digest>`). Each copy was rehashed against its source. Detailed checkpoint: `checkpoint_1510UTC.md`.
## Checkpoint — 2026-10-13 15:50 UTC; offline capture characterization and metadata contract update

The 15:02 native transaction and immediate restoration remain verified as documented in `checkpoint_1510UTC.md`. No live re-test or setting change occurred for the new offline analysis.

The capture characterization shows direction- and ear-dependent differences beyond shared gain or common latency. A global scalar fit leaves 29.831% candidate-relative L2 residual. Samples in the float WAVs lie on a PCM16 grid; the identical 450-ms quiet region has a 1/32768 peak and RMS about 1.42–1.44e−5. This shared quantized background makes raw spectral fits noise-sensitive. At 3× quiet FFT RMS, the raw common-filter diagnostic using available ears across at least two directions has 1,989 supported bins and 6.037% residual. The optional 28-ms window retains 93.998–100% of profile-difference energy across the five directions, but includes early reflections. Its 3× quiet-threshold common-filter diagnostic has 787 bins supported across at least two directions and 19.925% residual. Integer interaural correlation lag is zero and sub-sample changes remain below one sample; inconsistent/sparse phase estimates do not resolve ITD. These fit results do not establish listener physiology, perceptual improvement, or full internal decoder use.

Static metadata resolves typed properties and JSON names, including `Checksum` → `correlation_id`, capture `DateTimeOffset` members with property-specific `IsoDateTimeConverter`, and `Uuid` with `GuidConverter`. The activation converter writes a Boolean and the Guid-to-string routine is PDB-bound. This does not resolve effective formatting for fields with custom date attributes, final JSON output order, cache acceptance, or activation/persistence behavior. The global converter's UTC behavior must not be claimed as the output for those PHRTF dates.

Detailed sources: `native_capture_characterization_1540UTC.md`, `access_native_metadata_contract_1525UTC.md`, and `checkpoint_1550UTC.md`. The reproducible script and primary JSON/CSV/NPZ/PNG/SVG artifact hashes are recorded in the checkpoint; data remain at original private paths with no large-file copies. The native persistence lifecycle proposal is submitted to Dot, but no subsequent live test is released. Next work remains static activation/Guid conversion tracing.
## Checkpoint — 2026-10-13 16:17 UTC; lifecycle source review received, no new runtime release

The six-run native validation remains the only executed runtime sequence and retains its verified immediate restoration. The offline characterization and Access metadata updates are reflected in the current report and checkpoint 1550. The two phase-bound symbols in `native_capture_characterization_1540UTC.md` are now ASCII `+/-`; no numerical values changed.

Dot's `native_lifecycle_source_review_1615UTC.md` reviews a four-run OFF/ON sequence but explicitly says it has not been executed and the separate release record is absent. The old six-run approval does not authorize the new sequence. No new live operation was released or performed. Continue static activation and Guid serialization tracing while Dot reviews.

`native_capture_review_package_20261013.zip` was rebuilt with exactly the corrected report, `direction_summary.csv`, `frequency_comparison.csv`, `spectral_comparison.png`, and `optional_early_response.png`. Each member hash matched its source; the package is 1,242,086 bytes with SHA-256 `<private-artifact-digest>`. Detailed member hashes are in `checkpoint_1615UTC.md`.
## Checkpoint — 2026-10-13 16:30 UTC; getter deadlines added, runtime gate still absent

The completed six-run native validation remains the only executed runtime sequence. Dot's distinct four-run OFF/ON proposal remains unexecuted with no runtime release. The revised getter-deadline runner SHA-256 is `<private-artifact-digest>`; comparator hash `<private-artifact-digest>` and C++, C#, and native executable are unchanged from the reviewed source package.

Twelve focused cases passed, including mocked configured 10-second getter timeout handling through the callback and actual outer-finally, plus four benign fixture process transport cases. No real getter, BAT, audio, or property call occurred; timeout and property restoration were mocked. Getter process waits are bounded, but controller COM and pre-mutation backup preparation remain unbounded, so no hard three-minute whole-operation bound is claimed. Both proposed candidate captures use Dolby ON; the OFF checkpoint only reads the property.

Details are in `native_lifecycle_getter_review_1628UTC.md` and `checkpoint_1630UTC.md`. Dot review remains pending; no live operation has been released.