> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# CAPx WinMD interface metadata

The installed `CapxSettingsInterface.winmd` identifies runtime class **CapxSettingsInterface.CapxSettings** and default interface **CapxSettingsInterface.__ICapxSettingsPublicNonVirtuals**. The class's `InterfaceImpl` row carries `Windows.Foundation.Metadata.DefaultAttribute`; this establishes the default-interface association in metadata.

The interface GUID is **<identifier-removed>**, decoded from `Windows.Foundation.Metadata.GuidAttribute`. Its raw custom-attribute blob is `010028B3ACEC8987DC319B2F149248908B520000`. The interface is marked exclusive to `CapxSettingsInterface.CapxSettings` by `ExclusiveToAttribute`.

Interface methods below follow MethodDef row order (tokens 0x06000001 through 0x06000004). Parameter names, sequence numbers, and `[in]` flags come from the Param table. The Boolean return is a metadata return value; standard WinRT ABI projection represents it as the final output pointer, with HRESULT as the ABI method return.

| Method | Metadata signature | Input parameters | WinRT ABI projection |
|---|---|---|---|
| `IsCAPxSupported` | `Boolean(String)` | `audioRenderDeviceID`: `[in] String` | `HRESULT(HSTRING, [out] boolean*)` |
| `TryIncrementTransationCounter` | `Boolean(String)` | `audioRenderDeviceID`: `[in] String` | `HRESULT(HSTRING, [out] boolean*)` |
| `TryWriteDeviceProfileBlobToCapxPropertyStore` | `Boolean(String, UInt8[])` | `audioRenderDeviceID`: `[in] String`; `deviceProfileBlob`: `[in] UInt8[]` | `HRESULT(HSTRING, UINT32, BYTE*, [out] boolean*)` |
| `TryWriteSadBlobToCapxPropertyStore` | `Boolean(String, UInt8[])` | `audioRenderDeviceID`: `[in] String`; `sadBlob`: `[in] UInt8[]` | `HRESULT(HSTRING, UINT32, BYTE*, [out] boolean*)` |

`String` maps to `HSTRING`; `UInt8[]` maps to an input byte-array count and pointer under the standard WinRT ABI. The JSON preserves the metadata facts separately from this ABI projection. The projection does not establish implementation behavior or call sites.

Metadata identity: assembly `CapxSettingsInterface`, version `255.255.255.255`, MVID `<identifier-removed>`, Windows Runtime metadata version `1.4`; WinMD SHA-256 `<private-artifact-digest>`. Inspection used REA's target-free `inspect_managed_members` and direct `System.Reflection.Metadata` table reads. No target assembly was loaded or executed.

`CapxSettingsInterface.dll` and the WinMD were copied unchanged into `SpatialRendererProfileImport/binaries`; source and destination hashes match in the JSON.

Reproduction in PowerShell uses `System.Reflection.Metadata.MetadataReaderProvider.FromMetadataImage($pe.GetMetadata().GetContent())`, then reads `TypeDefinitions`, `TypeDefinition.GetCustomAttributes()`, `TypeDefinition.GetInterfaceImplementations()`, `MethodDefinition.GetParameters()`, and `Param.Attributes`. Dispose the provider, PEReader, and stream afterward.

No native calls, writer behavior, or native implementation were analyzed.
