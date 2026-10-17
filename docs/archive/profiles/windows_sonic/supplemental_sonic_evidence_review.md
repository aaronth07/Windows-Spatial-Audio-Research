> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Supplemental Sonic evidence review

This supplement narrows the previous audit's evidence-gap wording. The earlier review used the embedded-spatializer, override-gate, and process-presence bundles specified for that bounded pass. The additional source artifacts and static image inspection below were outside that initial input set; their earlier absence was not evidence that the claims were false.

## Factory path and constructor chain

`sonic_cache_source_initialize.txt` lines 241–269 show the selected branch loading `HrtfDspCpu.dll`, resolving `CreateHrtfDspFactory`, and calling it as `pFVar13(0, &local_60)`. The retained text is 39,709 bytes, SHA256 `<private-artifact-digest>`; `sonic_cache_artifact_manifest.json` records the same size/hash and `decompile_failed: false`.

`sonic_primary_lifetime_cpu_abi.txt` identifies `FUNCTION 180004c30` and gives its decompiler output and instruction listing (`ABI_INSTRUCTIONS 180004c30..180004d10`, lines 307–364). It shows incoming `R9D` retained as source count, local block size `0x400`, local flag zero, and the call to `0x180005034`. The call setup uses local pointers in `RCX`, `RDX`, `R8`, and `R9`; the incoming `RDX` and `R8` are not forwarded. The same retained listing connects `0x180005034` → `0x180005458` → `0x180003d54` → `0x18000752c` (see the direct-callee list and constructor call near lines 124 and 277). `sonic_primary_lifetime_manifest.json` verifies this ABI text at 15,621 bytes, SHA256 `<private-artifact-digest>`; the associated source hashes are HrtfApo `ffc12d088cc02441061cc2bb7f3b367fe8089662e0fd61ab98209ef6d43b6489` and HrtfDspCpu `4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9`.

This independently supports the dynamic factory call, the `0x180004c30` method's argument handling, its default block/flag values, and the constructor chain. The previously overlooked `sonic_cache_cpu_factory.txt` records `CPU_FACTORY_VTABLE 180141248 HrtfDspCpuFlex2Factory` at line 41 and `CPU_FACTORY_SLOT +18=180004c30` at line 45. `sonic_cache_artifact_manifest.json` pins this 22,129-byte text file at SHA256 `<private-artifact-digest>` with `decompile_failed: false`; its header identifies HrtfDspCpu SHA256 `4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9`. This closes the text-evidence gap for the factory class/vtable/slot mapping. The retained text lists the vtable entries but does not include the raw pointer bytes, and it supplies no Ghidra evidence ID for this mapping. Root is collecting a fresh read separately; byte-level confirmation is pending that retained read.

## Static API-set namespace

I ran `tools/inspect_spatial_api_set.py --help`, then used the documented positional-path invocation against `<local-path>`. The inspector reads the PE `.apiset` section and reports static namespace contents without loading a DLL or target. Its new output is spatial_api_static_namespace_supplement_20261014.json. It records schema version 6, System32 file SHA256 `<private-artifact-digest>`, and the exact contract entry `ext-ms-win-audiocore-spatial-l1-1-0` with empty `alias` and `host`. This confirms the report's static empty-host statement for the inspected installed schema. It does not establish process-local loader behavior or the active audio engine's selected CLSID.

## Contract name bytes in audioeng

The existing override-gate evidence record `ev_78a23a8219bc2df72a83f1893a7f58fd3b48370b83d7fcaab177b9d8e42a2605` is `audioeng.dll` SHA256 `<private-artifact-digest>`; it shows the call to `ApiSetQueryApiSetPresence(&DAT_1800fd2d0)`. The companion byte record `ev_f5b93ccc6ac9e51f49d61fab14c5a56ddddc81da3f310a690470c61fd43bfebe` reports `4600480000000000307a108001000000` at VA `0x1800fd2d0`.

I independently parsed PE section mapping from the inert project copy `binaries\audioeng.dll` and confirmed its SHA matches current `<local-path>`. Image base is `0x180000000`. The 16-byte structure is at file offset `0xfc2d0`; it decodes as `Length=0x46`, `MaximumLength=0x48`, and pointer `0x180107a30`. That pointed UTF-16LE data is at file offset `0x106a30`, bytes `6500780074002d006d0073002d00770069006e002d0061007500640069006f0063006f00720065002d007300700061007400690061006c002d006c0031002d0031002d003000`, and decodes exactly to `ext-ms-win-audiocore-spatial-l1-1-0`. The `audioeng` contract-string provenance claim is therefore confirmed.

## Remaining scope

The previous audit's three evidence-limit findings should be read as “not shown by the original bounded bundle,” not “disproved.” This supplement now confirms the factory method/constructor chain, static empty-host mapping, and literal contract bytes from additional retained artifacts. The `spatial_api_presence_20261014.json` observation remains explicitly process-scoped; neither it nor the static schema inspection identifies the active audio engine's runtime namespace state or effective override GUID. The factory-vtable identity/slot mapping at `0x180141248` remains the only unresolved factory-link detail in the specifically named source artifacts.

