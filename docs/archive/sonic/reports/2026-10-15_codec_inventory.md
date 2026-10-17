> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Installed media codec registration inventory

Read-only snapshot on 2026-10-15. The records below come from installed MSIX package manifests and the local HKLM Media Foundation Transform registry tree. They establish installed package declarations only; they do not establish that an encoder/decoder can currently be activated, is licensed, is selected by Windows, or works on another machine. `ActivatableClassId` values are WinRT class names, not CLSIDs. These package manifest declarations supplied no CLSID or COM interface GUID for the listed codecs. No matching DTS/Dolby-named value or key was found in `HKLM\SOFTWARE\Microsoft\Windows Media Foundation\Transforms`; the package-scoped codec extensions are distinct evidence from that machine-wide tree.

## DTS Sound Unbound

Installed identity: `DTSInc.DTSSoundUnbound`, version `2026.3.31.0`, x64. Package-relative paths below are under the installed package root.

| Manifest codec | Category | Activatable class | Binary | Input subtype GUID(s) | Output subtype GUID(s) | Binary version; size; SHA-256 |
|---|---|---|---|---|---|---|
| DTS:X for home theater | audioEncoder | `DTS.DtsxHomeTheaterEncoder` | `x64\DtsxHdmiEnc.dll` | `<identifier-removed>` | `<identifier-removed>` | `1.0.2408.12002`; 322,616 bytes; `<private-artifact-digest>` |
| DTSXDecoder | audioDecoder | `DTSXDecoder.DTSXDecoder` | `x64\DTSXDecoder.dll` | `<identifier-removed>`; `<identifier-removed>`; `<identifier-removed>`; `<identifier-removed>`; `<identifier-removed>`; `<identifier-removed>`; `<identifier-removed>`; `<identifier-removed>`; `<identifier-removed>`; `<identifier-removed>`; `<identifier-removed>`; `<identifier-removed>`; `<identifier-removed>`; `<identifier-removed>` | `<identifier-removed>`; `<identifier-removed>`; `<identifier-removed>` | File version not present in PE version resource; 1,490,432 bytes; `0fb48a12aef354016992e5fc2a34ea7ef4852711a5f48a66bdff07661285c4ca` |

The same manifest also declares DTS Headphone:X and DTS:X Ultra audio encoders in `x64\DtsHrtfEnc.dll`; they are omitted here because this inventory is limited to DTSXDecoder and DtsxHdmiEnc. DTS package manifest declares no codec-specific WinMD path or interface metadata for the two listed classes.

## Dolby Access

Installed identity: `DolbyLaboratories.DolbyAccess`, version `3.27.12250.0`, x64. Package-relative paths below are under the installed package root.

| Manifest codec | Category | Activatable class | Binary | Input subtype GUID | Output subtype GUID | Binary version; size; SHA-256 |
|---|---|---|---|---|---|---|
| Dolby Audio Processing | audioEncoder | `Dolby.DolbyAudioProcessingModule` | `DLLs\x64\DolbyAudioProcessing.dll` | `<identifier-removed>` | `<identifier-removed>` | `7.3.7.rel`; 4,384,256 bytes; `0fada40251880d03c657a357745d362799c93fb6024f733f1c8bf1057f5e4715` |
| Dolby Atmos for Home Theater | audioEncoder | `Dolby.DolbyMatEncoder` | `DLLs\x64\DolbyMatEnc.dll` | `<identifier-removed>` | `<identifier-removed>` | `1.0.2403.20001`; 138,168 bytes; `<private-artifact-digest>` |
| Dolby MAT Engine | audioEncoder | `Dolby.DolbyMATEngine` | `DLLs\x64\DolbyMatEngine.dll` | `<identifier-removed>` | `<identifier-removed>` | `3.1.8.rel`; 1,025,024 bytes; `<private-artifact-digest>` |

The package includes `Dolby.SpatialCodecs.winmd` and declares `Dolby.SpatialCodecs` app-service/background-task classes in its manifest; these are package application/service declarations, not manifest mappings from the three codec classes above to WinMD interfaces. The codec declarations provide no CLSIDs or declared interface GUIDs. Dolby Atmos for Headphones and built-in-speaker HRTF encoders are intentionally excluded from this inventory.

## Scope and evidence

- Package declarations were read from the installed DTS Sound Unbound and Dolby Access `AppxManifest.xml` files, including each codec's declared category, class name, binary path, input subtypes, and output subtypes.
- File sizes, PE version-resource values, and SHA-256 digests were read from each declared codec binary.
- The machine-wide HKLM Media Foundation Transform key names and their values were checked for DTS/Dolby-related names; none matched. No COM activation or codec invocation was attempted.
- No inference about runtime activation, device availability, license state, or functional output follows from these static registration records.

