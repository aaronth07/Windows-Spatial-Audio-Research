> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

The DTS SOFA candidate passes the installed encoder's native SPAC format validator. Its coefficient-loading and object-processing route is traced. Actual native use of the custom coefficients remains untested; no endpoint value or spatial provider was changed.

The candidate is 754,556 bytes, SHA-256 `<private-artifact-digest>`. Validation records the repeat conversion and native parser result. It derives from the working 710-direction,48 kHz,4,096-sample SOFA SHA-256 `<private-artifact-digest>`.

## Native loading and processing

Pinned `DtsHrtfEnc.dll` SHA-256 `90e62a7f6d044b43e53413f1f0933647725838f60904de37cd4b49f3852a604c`, Ghidra12.1.4. Full observations are retained in REA evidence; addresses below are virtual addresses at image base0x180000000.

| Function | Observed behavior |
|---|---|
|0x180003fc0|Activates endpoint effects context`08e54ab7-ff00-4b50-92bb-63d6dc1ef8ea`, opens User store read-only, reads runtime/device BLOBs and transaction counter.|
|0x180004360|Registers an endpoint-property-change notification for the device and context.|
|0x1800043f0|Requires the matching device/context/User store and counter property. A changed uint32 counter fetches/compares both BLOBs; changed data invokes0x180007948.|
|0x180007948|Coordinates reset/reinitialization using two waits on the DSP event and calls initialization0x1800606f0. This is not a bounded public API; the test writer needs a process deadline.|
|0x1800088d4|Processes up to128 dynamic objects through panning0x180010d34, generates node signals0x180011410, and invokes the FIR wrapper0x180015ef8.|
|0x18001846c|Duplicates each node signal for two convolution states; separately mixes auxiliary vectors into a dry stereo return.|
|0x1800196f8|Convolves state2*i to output0 and state2*i+1 to output1. This proves serialized FIRs are interleaved node-left/node-right.|
|0x180008254|Interleaves the two output buffers to PCM channels0/1.|

Initial raw data also has a property-bag path, but the endpoint route above is now established independently. A runtime BLOB write alone does not trigger the counter-specific handler. Native notification compares inequality, permitting restoration of the exact original counter after restoring its original BLOB.

Current read-only Windows COM getters returned the same bytes as the earlier registry extraction after removing its eight-byte wrapper. Runtime:189,308 bytes, SHA-256 `<private-artifact-digest>`; separate device processing:248 bytes, SHA-256 `<private-artifact-digest>`; counter 4282. Support property isVT_EMPTY, not evidence of CAPx support=true. All four Default-store getters returnedVT_EMPTY. Getter manifest, new backup.

## Conversion behavior

Converter preserves RPAN bytes, all native positions/radii/identifiers, the 96-byte response header except its updated size/sample count, and the separate device-profile BLOB. It replaces 22 directional FIR pairs with jointly interpolated SOFA responses, pads the origin's original FIR pair, and zeros all post-FIR auxiliary vectors. The first paired vector set is proven to feed the stereo dry return; the second set's full semantics remain unresolved. The origin's semantic label is unresolved; it is not silently assigned an HRTF direction. The identified dry gains are not interaural delays.

SOFA listener coordinates are forward/left/up. Native targets are converted from right/up/back to that basis as`[-z,-x,y]`, using the documented Windows object-position axes loads only the pinned inert DLL copy and accepts only three reviewed fixture hashes. It calls the analyzed SPAC parser atRVA0xbec8, without COM, streams, endpoint setters or binary modifications. Original SPAC, the FIR half-gain probe and the SOFA candidate each returned0. Parser acceptance does not establish DSP initialization or active rendering.

## CLI and next checkpoint

dts_profile_control.ps1 provides Backup, local Validate, Apply and Restore. Backup was actually exercised withSTGM_READ and verified byte-for-byte. Apply/Restore have not opened a writable store or run against Windows. Their common transaction was exercised against memory, including partial setter failures, readback drift, counter wrap, exact restore and idempotent restore. Writes are limited to the runtime BLOB and counter, in that order. Restoration never writes the modified profile back after an original-profile restoration error.

The separate capture engine reuses the existing native dynamic-object/WASAPI capture implementation, addresses only the named virtual endpoints, and checks exact provider/profile identity. Its x64 MSVC build passed/W4/WX; getter-only execution verified the retained Dolby personalized API hash. Recording through this adapted engine remains a live checkpoint. The original WSSAPI2SOFA source stayed byte-identical (`<private-artifact-digest>`).

At20:10:47Z the explicit virtual endpoint still reported active/default Dolby`<identifier-removed>`. The getter returned Windows Sonic`<identifier-removed>` and DTS Headphone:X`<identifier-removed>`; a repeat getter agrees. The earlier current-default-device guard failed, so future captures use explicit IDs and will not select a global default. Current formats.

The bounded native-use plan is the next approval checkpoint. Existing personalized Dolby bytes and all device processing remain the restoration target. No successful custom import, acoustic equivalence, license state or independent room control is claimed.
