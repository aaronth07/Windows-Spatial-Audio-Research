> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Property-test dependencies and rollback — 2026-10-13 13:22 UTC

The exact one-property test remains approved by <identity-removed> and paused for prior coordinator's evidence review. Fresh backups completed at 13:06:30 UTC after <identity-removed> closed Access and turned Atmos off. This assessment does not authorize a hive import, app-file replacement, cache synthesis, account change, or broader registry write.

The first useful live result is a bounded API SetValue/Commit, readback, immediate restore and readback while Access is closed and Atmos is off. It can establish persisted property access. It cannot establish native coefficient loading or rendering; those require later controlled validation. No object probe or unrelated audio harness is needed for this first result.

## Verified dependencies

`DolbyAudioProcessing.dll` SHA-256 `0fada40251880d03c657a357745d362799c93fb6024f733f1c8bf1057f5e4715` provides the concrete consumer path. `ConfigureEncoder` (`0x18012e640`) calls `RefreshAtmosForMonitorsStatus` (`0x18012bb00`) before applying DAP-VR init/runtime parameters. The refresh tests the DAFM init property and sets `this+0x738`; selector `0x18012bd10` uses that flag to choose DAHP or DAFM. Both use the Windows User system-effects/CAPX property framework. Earlier wording identifying DAFM as an alternative CAPX route was incorrect.

`ApplyModuleParams` (`0x18012dd90`, REA evidence `ev_3ae885b2a3eb7d75fa238b86b58e40c7e5468a3f54027856b5c3fefd8e669bc9`) obtains the selected key, reads IPropertyStore.GetValue, requires a nonempty VT_BLOB (0x41), and calls the selected module's SetParams. That branch bypasses `spSettingsMap`. Only an absent/unusable property reaches the DAHP map fallback with `dahp_v2_1_1_init_time_params` or `dahp_v2_1_1_run_time_params`. No Access hive open, JSON digest or settings.dat read occurs in this inspected function. SetParams success and the module's update log do not themselves prove coefficient decode: default tables can survive a failed tuning load.

Consequently, Access's settings.dat is not an input to the inspected successful User-property branch. This is a bounded static conclusion, not proof of independence across app activation, encoder creation, licensing or notification handling. Access can later regenerate endpoint properties from its own state, so an endpoint-only result must not be described as Access UI integration or restart persistence.

The PHRTF cache path is separate. Access generic-store constructor `0x182581660` resolves ApplicationData.Current.LocalCacheFolder.Path. `PhrtfStorage` reads/writes settings and config JSON at `0x18367a130`, `0x18367c9f0`, `0x18367cf20`; checker `0x18367d7c0` calls digest `0x18367ddd0`. Framework import resolution and matching public PDB corroborate MD5 over US-ASCII bytes, uppercase X2 output, code page 20127 with question-mark replacement fallback. JSON format argument is 1; writer defaults use two spaces and CRLF. The exact member order and final serialized text remain unresolved. The filename identifier is a boxed Guid from nested metadata, not demonstrated to be InstallationId. No cache file was generated or accepted.

## Offline hive results

User originals and working duplicates remain unchanged. No Windows hive load, replay, import or replacement was used. `settings_hive_inspection_20261013.json` records all hashes, sequences, checksums, bin/cell boundaries, typed-value counts and log hashes.

| Copy | Verified structure | Relevant decoded state |
| --- | --- | --- |
| DolbyAccess/settings.dat | 16,384 bytes; SHA-256 `<private-artifact-digest>`; REGF sequences 906/906; header XOR matches; 3 HBINs; 5 keys, 49 values | DapProfile JSON: Game; Performance true; Intelligent EQ Off; Volume Leveler false. Seven ordinary DAP preset entries. |
| DolbyAtmos/settings.dat | 8,192 bytes; SHA-256 `<private-artifact-digest>`; sequences 7/7; header XOR matches; 1 HBIN; 3 keys, 0 values | No values to propose editing. |

RegUnicodeString type 0x10c carries UTF-16 text and timestamp data; decoded JSON strings contain a trailing NUL. `HasSyncedProfile` contains the JSON string `"true"`, not a verified PHRTF activation flag. `AutoEnablement_DolbyAtmosForHeadphones_<device>` contains a timestamp; the selected virtual endpoint entry is `2026-10-13T06:27:02.6239094-05:00`. It is not an enable Boolean. Installation and licensing fields are retained privately and are not import targets. Scanned names and decoded text contain no explicit PHRTF/personalized/Rosella fields; opaque or unrelated encodings are outside that negative finding.

Access LOG2 contains valid HvLE transactions 902–905; Atmos LOG1 contains transaction 6. Both supplied hives are clean and have newer matching sequence numbers, so parsing these bases does not require replaying older logged transactions. Structure and hashes do not prove atomic copy provenance or a tested restoration procedure. The Access hive and both log hashes also match the fresh ordinary package backup, strengthening byte-identity evidence without establishing the user's copy method.

No justified hive edit has been identified. Inventing PHRTF keys, replacing InstallationId or toggling HasSyncedProfile would not follow a verified loader schema.

## Safest options

1. **Recommended:** review the fresh manifest, then perform only the approved property set/readback/restore with Access closed and Atmos off. Fresh API/registry baseline must match 14/22 bytes, all six User values must be backed up, and defaults must remain selected virtual endpoint. No sound or spatial enablement is included. Any access denial stops without elevation or another writer.
2. Continue cache-schema tracing offline if prior coordinator wants an app-managed import path before any write. Exact typed member mapping/order and PHRTF identity/config fields remain the blocker. This does not require editing the supplied hives.
3. After property persistence is established, separately review native load/object-rendering and Access persistence tests. <identity-removed>'s existing enable/disable BATs are the selected spatial switcher; validate their accepted subtype by getter readback and restore OFF. SoundVolumeView 2.36 predates the official 2.48 Windows 11 25H2 fix, so compatibility remains untested. No replacement/helper change is implied.

Rollback for option 1 is the exact fresh 14-byte baseline API payload using the same property store, followed by Commit and independent API/registry hash comparison. Verify all six User values, app files, defaults and zero spatial subtype. An exception after an attempted SetValue must still trigger normal API restoration. If restoration is denied or readback mismatches, retain the evidence and request prior coordinator's assistance; do not import a broad context export, full registry reference or hive. The private scoped export is evidence and contingency material, not an approved broad restore operation.

The candidate API is 62,791 bytes, SHA-256 `<private-artifact-digest>`. Its owned array has 15,691 total int32 entries including two outer header words; the 15,689 remaining words comprise 15,678 core plus 11 trailer. Selected framing/checksum ports pass the extended-trailer path; native DLL acceptance remains unperformed.

Evidence: `native_property_test_backup_20261013.json`, `settings_hive_inspection_20261013.json`, `dsp_branch_selection_20261013.json`, `private/rea_dsp_branch_20261013.json`, `access_digest_encoding_verified_20261013.json`, `ghidra_storage_trace.txt`, `native_import_verification_20261013.json`, and the revised `native_import_test_plan.md`. Full values, profile identity and coefficient data stay private.
