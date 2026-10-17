> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Runtime output-mode mapping

The mode that chooses initialized versus substituted room data comes from the runtime message's first optional `uint32`. The currently persisted normal DAHP runtime property contains9. This is a fresh read-only registry result, not an initialization default or an active-core readback. A separate DABS runtime property contains11 and must not be substituted for the headphone property.

Current saved-value decode, offline inspector, static property mapping, runtime decoder/caller functions, property selection functions and runtime application retain the evidence. All new Ghidra work used the private RoomPath project with pinned consumer DLL SHA-256 `0fada40251880d03c657a357745d362799c93fb6024f733f1c8bf1057f5e4715`.

## Message and setter

| Stage | Exact contract |
|---|---|
| Wire decoder0x18014e750 | Byte0 is presence. Zero consumes one byte and leaves mode absent; any nonzero requires four more bytes. LEuint32 atwire+1 is copied to decoded+4; decoded+0 records presence. Truncation returns status4. No mode enum bound is applied by this decoder. |
| Runtime caller0x1801234d0 | `ConfigureRunTime` initializes the decoded structure via0x180122b90, calls decoder0x18014e750 and, only on success, calls0x18011cd60. If mode is present it also retains its uint32 atmodule+0xb8. |
| Runtime applier0x18011cd60 | Decoded+0 absent skips output-mode update. Present plus decoded+8 absent calls0x180003da0(core,decoded.mode,0,NULL). Decoded+8 present instead supplies the optional mix matrix, count at+0xc, entries at+0x10, and required output-channel presence/value+0xcc4/+0xcc8. |
| Mode setter0x180003da0 | Stores normalized requested mode atcore+0x118; marks core+0x13b0 dirty when it changes. Allows0..13 and15, subject to init-mode coercion below.14 or other uint32 values normalize to1. |
| Application0x180003590 | Transfers requested core+0x118 to current core+0x11c when applying dirty state. The latter is read by the room selectors. |

The next wire field after the mode is the optional matrix: presence byte, LEuint32 entry count, then signed-int32 entries. Native bound is120 entries. Application requires supplied channel count when a matrix is present and limits it to10. Matrix values are subsequently clamped/scaled in the setter. These details explain the different blob lengths; the new inspector decodes only the evidenced mode prefix and does not claim full-message validation.

Initialization mode is a different integer atcore+0xc. Under init mode0/1, output modes8/9/12/13 remain unchanged. Under init mode2,8/9/12/13 normalize to1,10→3,11→6,15→1. The personalized init payload's optional integer0 is not proof that current output mode is0. Setter/current-state evidence and full selection predicates remain in native_path_selection.md.

For eligible inputs in core0x180007d10, current output modes8/9 use initialized room/PCA.12/13 substitute the built-in room only when core+0x28 iszero; otherwise retain initialized room.0x18002c2b0 uses initialized room for8/9 and substitutes it for other modes. Other gates, input descriptors and transition state still apply. No enum names such as a Windows object category are assigned to8/9/12/13 by this pass.

## Property and serializer bridge

Consumer0x18012dd90 (`ApplyModuleParams`) calls key selector0x18012bd10 with module-kind1 forDAP_VR and parameter-kind1 forRuntime. It obtains a property-store value and accepts `VT_BLOB`65 with nonzero length/pointer, then dispatches the raw bytes and parameter-kind to `SetParams`0x180124d00. Runtime kind selects0x180125720 (`SetRunTimeParams`), which copies the supplied raw bytes into module+0x90 with length+0x98 and calls0x1801234d0. This is the exact property→wire decoder→mode setter bridge; no Access GUI producer or CLR enum label is inferred.

Normal rendering selection is parent+0x2c:0=DAHP,1=DABS, as diagnostic strings establish. The separate parent+0x738 flag selects the DAFM/Atmos-for-Monitors property family, not CAPX availability. Windows User-store diagnostics mention CAPX for the getter framework independently of this flag.

| Rendering/property family | Static key address | PROPERTYKEY |
|---|---|---|
| Normal DAHP Runtime | 0x1803ecbc8 | `{1b4dab55-b1fb-4d8c-8317-f2d4a96efbb8},2`, `PKEY_DAPVR_RUNTIME_DAHP` |
| Normal DABS Runtime | 0x1803ecb98 | SameGUID,5, `PKEY_DAPVR_RUNTIME_DABS` |
| DAFM DAHP Runtime | 0x1803ecb68 | `{<identifier-removed>},2` |
| DAFM DABS Runtime | 0x1803eca48 | SameDAFM GUID,4 |

Getter failure can fall back to a supplied settings map for normal DAHP.0x18012dd90 selects `dahp_v2_1_1_run_time_params` (pointer slot0x1803ecae8) forRuntime or `dahp_v2_1_1_init_time_params` (slot0x1803ecb00) forInit, extracts bytes and dispatches them through the same module setter. The exact retained User-store bytes can therefore differ from a later effective message if another supplied source or notification overrides them. Actual settings-map content and active parent flags were not read.

## Current persisted values

At2026-10-14T19:43:51Z the exact named endpoint's normal Dolby User registry key was opened read-only:

`HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\MMDevices\Audio\Render\{<identifier-removed>}\FxProperties\{45da5c30-2837-4ac4-b1e2-50acc3865974}\User`

| Property | Bytes / SHA-256 | Mode prefix |
|---|---|---|
| DAHP,2 |369 / `<private-artifact-digest>` | Present,9 |
| DABS,5 |601 / `<private-artifact-digest>` | Present,11 |

Both values match their retained Oct13 copies. Registry representation starts with the existing eight-byte envelope `4100000001000000`; mode presence is registry+8, value is LEuint32 at+9. The API blob omits that envelope and starts at presence. The saved-value inspector verifies the envelope and canonical0/1 presence; the native decoder accepts any nonzero presence. The current snapshot/manifest files preserve all original bytes. No User value was set or committed.

This proves the currently persisted normal headphone message requests9, which would remain9 under init mode0. It does not prove native setter execution, the currently effective output mode, source/category assignment, selected room pointer, near-profile send, or live contents of the static built-in structure. No conclusion is drawn from measured tail length.

## Narrow getter proposal and limits

The existing validated read_effect_property.ps1 supplies an evidenced getter contract: exact intended endpoint, normal context `{45da5c30-2837-4ac4-b1e2-50acc3865974}`, `IEffectSystemStore.OpenUserPropertyStore(STGM_READ=0)`, `IPropertyStore.GetValue`. A later primary-owned getter may request only `{1b4dab55-b1fb-4d8c-8317-f2d4a96efbb8},2`, require successful HRESULT andVT_BLOB65, preserve raw API bytes/hash and decode the prefix above. Optional DAFM,2 should be reported separately if specifically needed, never treated as interchangeable. Do not change routing or call setters. No such COM/property-interface activation was performed in this pass because consumer DLL execution was excluded.

That getter can confirm current property-store bytes and API/registry agreement. It cannot read current core+0x11c or prove an update reached the active stream. No supported getter for that effective core value has been established; the internal module+0xb8 stored mode also has no proved external read contract. A later authorized behavioral comparison can test processing changes without pretending persisted mode equals active mode.

All15 test groups passed onPython3.8, including the existing12 tests, invalid/truncated envelopes, optional uint32 extremes, mode coercions, saved-key decoding and CLI parity. New scripts compiled and executed. Second-pass review corrected the DAFM flag label and enforced integer initialization-mode inputs. Headless runs completed without decompile failure. Only project evidence files and private Ghidra artifacts were written; no Dolby/native audio execution or configuration change occurred.
