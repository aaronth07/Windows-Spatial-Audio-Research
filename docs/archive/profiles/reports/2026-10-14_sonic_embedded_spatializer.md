> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Installed Sonic CPU spatializer uses embedded data

The default CPU implementation constructs its spatializer from fixed tables inside `HrtfDspCpu.dll`, rather than using the previously mapped primary HRIR dataset in its CreateDSP call. Its actual direction lookup and processing consume those embedded tables. Replacing the registry's `DefaultHrtfs.bin` path is therefore not an evidenced custom-HRTF import into this implementation. The service's explicit-HRIR parser remains real, but its existence does not establish that the selected Flex2 spatializer uses those samples.

The current installed System32 hashes were rechecked and match the inert analysis copies: HrtfDspCpu `4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9`, HrtfApo `ffc12d088cc02441061cc2bb7f3b367fe8089662e0fd61ab98209ef6d43b6489`, audioeng `<private-artifact-digest>`. This is static shipped-code evidence, plus a separately recorded read-only core availability query. Sonic was not selected and no native Sonic stream/profile test occurred.

## Factory and embedded constructor

The prior factory ABI audit established CreateHrtfDspFactory(0)→actual HrtfDspCpuFlex2Factory vtable0x180141248, slot+0x18→0x180004c30. The method ignores its incoming primary/environment configuration pointers, preserving only source count and its output pointer; it constructs HrtfDspEngine with block0x400 and flag0. That calls0x18000752c, which writes `AudioDsp::FlexSpatializerHrtfEngineInterface::vftable`0x180141348 and initializes fixed descriptors.

Helpers0x1800097d8 and0x180009834 take a static descriptor's function pointer, dimensions and stride, and store the returned table pointer.0x180009784 uses the fixed360×181 descriptor. The getters below return the named embedded base plus a row-stride displacement; they perform no file/property/mapped-interface read.

| Descriptor | Getter | Embedded base | Recorded descriptor dimensions |
|---|---|---|---|
|0x1801423e8|0x1800068d0|0x1801b2df0|360,360,181; two-byte elements|
|0x180142388|0x1800068f0|0x180172970|16,16,4096; float32|
|0x180142320|0x180006910|0x180152430|16,16,1; float32|
|0x180142348|0x180006930|0x180172470|20,20,16; float32|
|0x180142368|0x180006950|0x1801b2bf0|16,16,8; float32|
|0x180142408|0x180006970|0x1801b29f0|8,8,16; float32|
|0x1801423c8|0x180006990|0x180152470|2048,2048,16; float32|
|0x1801423a8|0x1800069b0|0x1801b2970|16,16,2; float32|
|0x180142300|0x1800069d0|0x180142430|8192,8192,2; float32|

These are observed descriptor words, not automatically named HRIR lengths, physical ears, basis orders or response units. The complete REA evidence retains constructor, byte reads, getter bodies and subsequent processing. Snapshot. Constructor evidence`ev_3245f597cc11f0f72f7bbe33e0904882e6fba948054db19c7df2af66bfd753d9`.

## Direction and output consumers

0x180008778 computes azimuth/elevation from each source's xyz values, maps them onto the360×181 indices, reads a signed16 index from the embedded lookup table and copies16 float weights from the selected embedded row into per-source working data. It then applies source gain and a blend with the retained center/default vector. The decompiler's clamp helper arguments are imperfectly recovered; exact source scalar labels beyond xyz/gain/blend are not inferred. Evidence`ev_7be1f2251ff6fd54e26df63547c1e006ec234bbc54c154a5f592a0210a57d43e`.

0x180008b28 processes active source buffers/parameters and invokes that lookup.0x180008e68 uses the resulting working data in matrix operations.0x180008f9c also supplies the constructor's fixed descriptors atengine+0x98/+0xc8/+0xf8/+0x128 to its transform/matrix/filter stages. Helpers0x180009464/0x1800096c0 select an owned working pointer when present, otherwise the descriptor's retained table pointer, and invoke0x180011934 with the recorded dimensions/strides. This positively connects fixed coefficient data to the inspected processing path; it is not just unused data in a DLL.

The engine-interface vtable's processing slot+0x20→0x180008210 drives0x180008e68,0x180008f9c and0x1800091c8. The last routine copies/interleaves the resulting two output buffers. The native loop still accepts arbitrary object positions; this finding concerns listener data selection, not a reduction to a channel bed.

## Override availability

In audioeng0x18005a228, the override GUID is initialized to zero and TryGetHrtfEngineOverrideCLSID is called only if0x18005c604 returns true. Assembly for0x18005c604 queries the API-set contract `ext-ms-win-audiocore-spatial-l1-1-0`, with a separate output Boolean omitted from its decompiler signature. It caches1 for present or2 for absent. The contract's Unicode name was verified from the binary, rather than guessed from its import.

The installed static API-set namespace maps that contract to an empty host. A fresh call to the same core `ApiSetQueryApiSetPresence` returned raw0 and outputBoolean false in the research process. Presence observation, static inspector, getter. The call accesses no audio DLL or endpoint. Microsoft's API documentation closes the immediate factory/interface questions. A fresh byte read at0x180141248 confirms slot+0x18 points to0x180004c30 (`ev_9fb1671d73718b4d779ee5d7499b291db60b970bc229b0f12fe468e95ccb7d32`). The exported `CreateHrtfDspFactory` at0x180003110 accepts only argument0; every nonzero value returns0x8000ffff. A hidden alternate selected by changing that argument is therefore excluded for this export/version (`ev_c19149a628f6260f58a19e1559eb97861b0c380f12b04ab552dabb166ef22eb5`).

The actual COM engine vtable at0x1801411e8 has IUnknown plus slots+0x18/+0x20/+0x28/+0x30/+0x38/+0x40/+0x48. QueryInterface0x180004d90 exposes only IUnknown and the one IHrtfDsp identity. Slots+0x18/+0x20/+0x30 return0 without reading arguments; +0x38 is a no-op. Processing+0x28→0x180004050 copies per-source position, gain, source buffers and other fixed-size parameters into owned working records, then calls the embedded spatializer's processing slot+0x20. It accepts no primary dataset or variable coefficient array. +0x40→0x180004330 disables a source through the embedded interface+0x10; +0x48→0x180004380 forwards the embedded interface+0x30, which returns0. Evidence IDs `ev_78d6fc76…`, `ev_941b63d9…`, `ev_a95adc9c…`, `ev_e696b9c2…`, `ev_798486d0…` retain these exact bodies and bytes.

The constructed embedded interface0x180141348 likewise offers destruction, no-op, source disable, reset of owned working buffers, processing, output-buffer clear, and constant-zero getter. Its reset0x180008320 clears working buffers; it does not replace the retained fixed descriptors. All seven method bodies were inspected. These interfaces provide no demonstrated coefficient-replacement operation on this implementation. This is a bounded architectural conclusion for the inspected default factory and version, not a claim about every Windows implementation, future update, or a separately supplied spatial-audio renderer.

Microsoft's object-rendering documentation verifies the earlier factory text, static API-set mapping and exact contract UTF-16 bytes. The original bounded review's missing-bundle findings were evidence-input limits, not contradictions. The fresh vtable read above also closes its remaining pointer-byte gap. No Sonic endpoint write or Sonic playback was needed for this static interface audit.
