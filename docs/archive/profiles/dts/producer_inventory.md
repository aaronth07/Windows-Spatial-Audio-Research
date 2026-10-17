> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# DTS app-to-renderer configuration bridge inventory

Collected 2026-10-14 through read-only static inspection of the installed DTS Sound Unbound package and filenames in its package-owned LocalState. The strongest producer lead is the registered `CapxSettingsInterface` contract: it declares methods to write device-profile and SAD byte arrays to a CAPx property store. The app executable contains these method names and the two renderer BLOB key names. This supports a possible producer bridge, but does not establish a caller-to-renderer call sequence.

The installed package is `DTSInc.DTSSoundUnbound` version `2026.3.31.0`, package full name `DTSInc.DTSSoundUnbound_2026.3.31.0_x64__t5j2fzbtdg37r`. Its manifest SHA-256 is `<private-artifact-digest>`. Artifact paths, hashes, lengths, file versions where present, and WinMD MVIDs are in `producer_inventory.json`.

## CAPx writer contract

`AppxManifest.xml` registers `CapxSettingsInterface.CapxSettings` from `CapxSettingsInterface.dll` as an in-process class. The corresponding WinMD declares `IsCAPxSupported(string)`, `TryIncrementTransationCounter(string)` (metadata spelling), `TryWriteDeviceProfileBlobToCapxPropertyStore(string, byte[])`, and `TryWriteSadBlobToCapxPropertyStore(string, byte[])`. `DTSSoundUnbound2.dll` contains these method names and both `RuntimeSettingsBLOB` and `DeviceProfileSettingsBLOB`. `x64\DtsHrtfEnc.dll` contains both BLOB names in UTF-16LE at byte offsets 432768 and 432808; parent static analysis separately found the renderer reads those names during engine initialization.

`CapxSettingsInterface.dll` imports `MMDevAPI.DLL` (ordinal 17 in the inspected import listing), COM helpers, and property-variant helpers. `AudioClientInterfaceProxy.dll` imports `MMDevAPI.DLL`, `PROPSYS.dll`, and `InitPropVariantFromCLSID`. The app PE import table does not list a direct `CapxSettingsInterface.dll` dependency. These details identify a plausible settings/property-store API surface, not proven writes or call flow.

The WinMD methods have RVA 0, so metadata provides declarations rather than implementation bodies. `DTSSoundUnbound2.dll` is a native x64 PE with no CLI data directory according to REA. The inspected native DLLs likewise have no CLI data directory; their paired WinMD files provide metadata declarations. No target assembly was loaded or executed, and no active REA document was opened or switched.

## Observable component identities

The package exposes distinct naming surfaces: `HeadphoneEQConverter`, `EagleToI3DABlob`, `GetI3daBlob_GPEQDisabled`, and `db_hpeq_*` identify a headphone-EQ-related API/data naming surface. `RuntimeSettingsBLOB`, `DeviceProfileSettingsBLOB`, and `*.SPAC.crypt` do not themselves identify listener-personalization content. `x64\DTSXDecoder.dll` is registered in the package manifest as an `audioDecoder`, and the decoder contains `SpatialAudio` strings. These are names and registrations only; no implementation behavior or import feasibility is inferred.

The app contains `Endpoint` and `PropertyStore` strings, and `resources.pri` contains Endpoint resource-index matches. Byte offsets and selected counts are recorded in JSON. These string hits do not prove the registered classes write to a particular endpoint or property key.

## Package-owned LocalState

The DTS app's LocalState directory exists. Its filenames include three `*.SPAC.crypt` files (567984 bytes total) and twelve `db_hpeq_*` files (11584 bytes total). Only names, counts, and aggregate sizes were read; file contents were not opened.

## Reproduction

Run in PowerShell. These commands query package identity and artifact metadata, and locate the documented package-owned filenames. String offsets in the JSON were obtained by finding byte sequences in each listed artifact's raw bytes, using ASCII or UTF-16LE as recorded.

Historical workspace command omitted; use the public [example guide](../../../../examples/README.md).

The recorded byte offsets use zero-based indexing into file bytes. PE import inspection used `<local-path>`; `dumpbin.exe` was unavailable on PATH. REA target-free metadata inspection used explicit package file paths.

No app launch, COM/WinRT activation, playback/capture, registry or configuration mutation occurred. No entitlement credentials or secrets were read. No implementation decompilation or call graph was performed. The evidence does not establish which key a caller supplies to the CAPx writer, whether a write succeeds, or whether listener-profile data can be supplied.
