> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Consumer PHRTF test proposal — 2026-10-13

Status at 13:28 UTC: the previously approved property-only round trip passed. prior coordinator reviewed the complete backup and directed execution subject to immediate preflight. Fresh backups completed again at 13:27:59 UTC. The exact candidate API/registry bytes read back after SetValue/Commit; the original bytes were immediately restored in finally and independently read back. All six User values, app files, defaults and Atmos-OFF state verified afterward. No spatial command or audio stream ran. This establishes property persistence only. See `property_round_trip_20261013.json` and `checkpoint_1328UTC.md`. The dependency assessment records the earlier 13:22 preparation state.

Hypothesis: storing the owned PHRTF coefficient array and version in tuning-v2 of the current Dolby headphone initialization property causes the consumer DSP to deserialize and use the personalized room/PCA tables. Acceptance, sound changes, and Dolby Access persistence are separate results.

Exact target:
```text
Endpoint: {<identifier-removed>}
Context:  {45da5c30-2837-4ac4-b1e2-50acc3865974}
Property: {1b4dab55-b1fb-4d8c-8317-f2d4a96efbb8},1
Type:     VT_BLOB / REG_BINARY
Registry:
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\MMDevices\Audio\Render\{<identifier-removed>}\FxProperties\{45da5c30-2837-4ac4-b1e2-50acc3865974}\User
```

Approved operation: obtain the existing IAudioSystemEffectsPropertyStore for that endpoint/context, open its user store with STGM_READWRITE, and SetValue plus Commit for only the property above. The Windows API sends the initialization payload WITHOUT the eight-byte registry serialization envelope. No automatic fallback to reg.exe, elevation, ownership changes, or permission bypass is allowed after an access denial. The approved preflight opened the STGM_READWRITE interface with `S_OK`, but did not call `SetValue` or `Commit`; it does not establish that writing will succeed.

| Representation | Before | Proposed |
| --- | ---: | ---: |
| Registry value including envelope | 22 bytes | 62,799 bytes |
| API VT_BLOB payload | 14 bytes | 62,791 bytes |
| Tuning-v2 coefficient array | absent | 15,691 owned signed int32 values |
| Tuning-v2 version | absent | 5 unchanged ASCII bytes |
| Other fields | Boolean true/false/false, integer 0; v1/v3 absent | byte-identical |

Registry baseline SHA-256: `<private-artifact-digest>`.
Registry candidate SHA-256: `<private-artifact-digest>`.
API candidate SHA-256: `<private-artifact-digest>`.
Owned file SHA-256: `<private-artifact-digest>`.
Private candidate directory: `<local-path>`.
The .reg draft is a review artifact, not the preferred application method.

The current process token is not an administrator (read-only membership check). Microsoft documents administrator access for a writable user effect store. The approved `STGM_READWRITE` open returned `S_OK`; a setter may still fail. Any access denial stops execution without elevation, ownership changes, permission bypass, or another write path. The earlier 12:45 incomplete backup is retained as historical evidence. The current complete backup supersedes it for preparation, subject to fresh baseline checks immediately before execution.

Static basis: consumer DolbyAudioProcessing.dll identifies this key as PKEY_DAPVR_INITTIME_DAHP, opens the matching Dolby spatial-plugin context, reads VT_BLOB, and dispatches init-time changes to CDapVRModule::SetParams -> SetInitTimeParams -> ConfigureInitTime -> DecodeMessageTuning. This User property path itself uses the Windows CAPX/system-effects framework. The alternate key `{<identifier-removed>},1` is the DAFM/Atmos for Monitors branch, selected by `RefreshAtmosForMonitorsStatus` at `0x18012bb00` and flag `this+0x738`; it is not selected by CAPX availability. Its latest read-only value was VT_EMPTY. `ConfigureEncoder` at `0x18012e640` refreshes that flag before `ApplyModuleParams` at `0x18012dd90`. The latter prioritizes a nonempty VT_BLOB from the selected endpoint User property over `spSettingsMap`. Read-only observations while Atmos is off do not prove a running encoder's branch. The setter's successful HRESULT or SetParams return alone cannot prove decoding: failed coefficient loading can fall back to existing/default tables.

Before the approved test:
1. Re-read console and multimedia defaults and active/default spatial subtype. Abort unless both defaults are the exact selected virtual endpoint endpoint and active/default spatial subtype are both zero (<identity-removed>'s current OFF baseline). Require Access to remain closed. The initial persisted-property test needs no playback, spatial enablement, or object probe.
2. Re-read the API property and registry value; verify 14/22-byte baseline hashes and exact equality after stripping the envelope. If another process changed any field, stop and regenerate the proposal from that state.
3. Save fresh private copies of the exact property payload, registry value, the existing User context's value kinds/bytes, and a scoped context export. Independently read/hash each backup. Keep the full supplied registry reference offline; it is never a restore source for this test.
4. Preserve Access-closed/OFF state. The copied Access hive records Game, Performance=true, Intelligent EQ=Off and Volume Leveler=false; this is persisted data, not a current UI observation. Privately copy and hash package LocalCache settings/profile files and Settings/settings.dat plus LOG1/LOG2; record LocalState/settings.dat and PHRTF file absence. Any incomplete required backup stops execution. The first property test will not create or edit app files.
5. Revalidate the candidate against the owned source and prove the only wire-field difference is tuning-v2. Save a timestamped operation manifest with the approved target, both hashes, process/session identifiers, backup paths, and input/capture choices.

Test sequence after approval:
1. After prior coordinator reviews the evidence, with the approved existing property store set and commit only the candidate VT_BLOB. Read it back through the API and registry; require exact expected bytes. Immediately restore the fresh 14-byte API baseline, commit, and verify the 14/22-byte API/registry hashes. Compare all six User values, app files, defaults and zero spatial subtypes against the fresh backup. Save HRESULTs and hashes, keeping coefficients private. This first test runs with Access closed and Atmos off, without playback. Native acceptance and audio use remain pending even if property persistence succeeds.
2. A later native-rendering test requires its own reviewed source/capture plan. Enable Atmos only using <identity-removed>'s existing `<local-path>`, after confirming its hash from the inventory. Execute from that repository directory and verify the same default endpoint and Dolby subtype by readback. Restore OFF using `Disable spatial sound.bat` and verify zero subtype; a BAT's printed success is not proof. Local SoundVolumeView 2.36 predates the documented 2.48 Windows 11 25H2 fix, so failed enable/readback is a compatibility blocker, not permission to replace it. Use bounded native Windows Spatial Sound objects on the SAME playback endpoint: front, left, right, rear, and elevated positions, one at a time, identical 48-kHz mono stimuli, gain bounded to avoid clipping. Capture only selected virtual endpoint through WASAPI. No routing, default-device, service, driver, security or endpoint-format changes. The probe is not built or run yet; source, accepted stream format, positions, duration and gain must be recorded and reviewed before execution.
3. Compare baseline/candidate/restored recordings with alignment and repeat captures. Baseline and restored output must agree within observed repeated-run variation; candidate differences must exceed it consistently across positions. Ordinary stereo playback cannot establish personalized object rendering. A real 5.1/7.1 or Atmos source may supplement this only with verified channel/decoder/render conditions; WASAPI alone does not decode Atmos.
4. Observe native error/success diagnostics if available without new persistent logging settings. Seek direct coefficient-load evidence. An output difference alone remains evidence of changed processing, not proof that every owned coefficient was used.
5. Restore the exact fresh baseline property immediately after any bounded comparison, including on exceptions. Read through both API and registry and verify hashes, defaults and relevant app file hashes. For a later rendering test, capture the restored Dolby control before disabling Atmos and verify the final OFF subtype afterward. Never restore the obsolete Atmos-on baseline from earlier checkpoints.
6. Only after acceptance and restore are verified, propose a separate Dolby Access launch/close and persistence test. App activity could overwrite the endpoint setting; do not infer cache/UI integration from an endpoint-only result. Do not patch account, entitlement, server-response, service, or license fields.

Abort/restore conditions: unexpected baseline, wrong endpoint/subtype, access denied, missing verified backup, wrong property type/length, failed commit/readback, decoder errors, silence/clipping, unrelated playback contaminating capture, app overwrite, routing drift, encoder failure, or restore mismatch. Stop only the probe's own streams, restore the one approved property, and verify. If normal API restore fails, report the exact failure and obtain assistance; do not escalate permissions or import any broad registry backup.

Result criteria:
- Persisted property: exact API and registry readback.
- Native acceptance: verified tuning decode/load without fallback.
- Actual use: reproducible object-rendered change with baseline/restored controls and coefficient-load evidence where available.
- Dolby Access integration: separately verified UI/configuration and persistence.
The user's requested working import is complete only when acceptance and actual use are established; the offline candidate currently satisfies none of those live result criteria.

Microsoft API references: APO settings framework (historical reference unavailable)

