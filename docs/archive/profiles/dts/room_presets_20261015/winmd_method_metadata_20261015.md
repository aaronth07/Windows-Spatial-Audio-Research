> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# DTS SAD API WinMD metadata check

## Result

No installed WinMD in DTS Sound Unbound package `2026.3.31.0` declares any of these names: `GetRuntimeSettingsBlobAsync`, `GetBlobNamesForSADOption`, `GetBlobNameForSADOption`, or `UpdateSADOption`. Therefore the package WinMD metadata does not provide corresponding runtime class/interface signatures, parameter names, or GUID attributes to report.

An exact ASCII string search across the six package WinMDs and the focused DTS DLL set found those four names only in `DTSSoundUnbound2.dll`. This is native application string evidence, not a WinMD declaration or proof of call flow. Existing app DLL identity: 17,617,920 bytes, SHA-256 `374fd16ed693cf79fc63e4c401e97a9a4e17f462c986e2579bab2eedbf6ddac5`.

## WinMDs inspected

REA `inspect_managed_members` used explicit package paths; metadata state was complete for each artifact. No code was loaded or executed, and no active native REA target was opened or switched.

| Package WinMD | Bytes | SHA-256 | MVID | Evidence ID |
|---|---:|---|---|---|
| `AudioClientInterfaceProxy.winmd` | 5,632 | `<private-artifact-digest>` | `<identifier-removed>` | `ev_174ff6b14019658f26e21a2a3a08f8ea2e450000c2fc093893fe154d83b85c49` |
| `CapxSettingsInterface.winmd` | 4,608 | `<private-artifact-digest>` | `<identifier-removed>` | `ev_cdef43cb252df57cf9e21257febe83d0cea6c7a1a745347ae196ea1f5a66f768` |
| `DTSDsecProxy.winmd` | 5,632 | `<private-artifact-digest>` | `<identifier-removed>` | `ev_7e49048795422e9e0b6fb70e6211ed5aa4227cf92e6dc11836ede1b98c25bc08` |
| `HeadphoneEQConverter.winmd` | 4,608 | `<private-artifact-digest>` | `<identifier-removed>` | `ev_5deb9e6a0903e513e4c884866a69dfcc4324ca0a7a8a5d5c8538d5ab947bfe7f` |
| `LicenseService.winmd` | 14,336 | `<private-artifact-digest>` | `<identifier-removed>` | `ev_df56e641985a264aba1039f0ccc0cb5be6aada7b4a4d249d7753b138b728098b` |
| `SpatialSoundPlayer.winmd` | 5,120 | `<private-artifact-digest>` | `<identifier-removed>` | `ev_9b43f45184e750abfa9cddc5ffcb4d43fe145acf74aecffaf8d5c06edafbdf18` |

`SpatialSoundPlayer.winmd` declares only SpatialEditor stream, position, and sound-file methods. `CapxSettingsInterface.winmd` is the SAD-related writer contract already inventoried separately (`TryWriteSadBlobToCapxPropertyStore`); it does not declare the four queried names.
