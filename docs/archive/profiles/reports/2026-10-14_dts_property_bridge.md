> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

The installed DTS settings component writes native endpoint property blobs. The decoder consumes SPAC/RCHN response data and an RPAN object-panning structure. A custom listener import still needs the exact configuration-to-encoder property-bag bridge, complete rendering semantics and a controlled use experiment. No native DLL, DTS app or audio stream was executed in this pass; live endpoint registry values were read only.

## Endpoint writer

`CapxSettingsInterface.dll`, SHA-256 `<private-artifact-digest>`, implements the WinMD default interface recorded in capx_abi.json. The default-interface vtable at0x18000b648 has six standard WinRT slots followed by these four methods:

| Method | ABI entry | Implementation |
|---|---|---|
| IsCAPxSupported |0x1800021d0|0x180001750→0x180001620|
| TryIncrementTransationCounter |0x180002200|0x180001820|
| TryWriteDeviceProfileBlobToCapxPropertyStore |0x180002230|0x180001a20|
| TryWriteSadBlobToCapxPropertyStore |0x1800022a0|0x180001c10|

All use0x180001590→0x180001400 to activate `IAudioSystemEffectsPropertyStore` IID `<identifier-removed>` on the supplied render-device ID. Activation parameters contain VT_CLSID context `08e54ab7-ff00-4b50-92bb-63d6dc1ef8ea`. The implementation uses ActivateAudioInterfaceAsync with a five-second completion wait, then opens its User store with access2 (STGM_READWRITE).

Exact property keys, all PID1:

- Device-profile BLOB: `{<identifier-removed>},1`.
- SAD/runtime BLOB: `{<identifier-removed>},1`.
- Transaction counter: `{<identifier-removed>},1`, VT_UI4.
- CAPx support query: `{<identifier-removed>},1`, expects VT_BOOL.

The writers pass the supplied byte count/data directly to IPropertyStore::SetValue as VT_BLOB. They first read the transaction-counter key but do not increment it themselves. Increment is a separate method. The ABI methods return HRESULT0 while putting operation success in the final Boolean output; checking HRESULT alone would therefore conceal failure. No Commit call appears in these selected implementations. A replacement writer must investigate change notification/transaction sequencing rather than adding an unsupported broad reset.

Microsoft documents the store and its restricted audio-device-configuration capability, and states that OpenUserPropertyStore opens only existing context stores and limits nonadministrators to read access: IAudioSystemEffectsPropertyStore records their types, sizes/hashes and the counter4282. The runtime registry representation is189,316 bytes, SHA-256 `<private-artifact-digest>`. Its first eight bytes are the observed registry wrapper words65 and1; the remaining189,308 bytes begin SPAC. This wrapper is storage data, not a header to feed to the renderer's byte-array decoder.

The extracted runtime SPAC has SHA-256 `<private-artifact-digest>`. It contains RCHNv2,48000 Hz,23 response positions, mode mask1 and one1024-sample table with46 FIR arrays. RPAN has group counts16/6/1/0,23 coordinate triplets and23 integer identifiers. The first position is(0,0,-1); later positions include radius0.25 and an origin position, so normalizing every point to unit radius would discard supplied information. Group meaning, complete coordinate convention and origin-channel behavior are not inferred from the arrays. The current device-profile registry representation is256 bytes, SHA-256 `<private-artifact-digest>`, with an RDEV/RGPQ payload; this is a separate processing stage.

The current dataset differs from the embedded14-position/two-mode default. A converter must preserve or deliberately account for the selected current carrier, its object-panning metadata and its additional coefficient vectors. It must not assume that an embedded fallback was the active dataset.

## Validation and coefficient consumption

Native RCHN validator0x18000bbb4 checks sample rate24000..192000 and count1..999. The selected response table permits lengths1..999999 and has branch-dependent auxiliary-header checks. These format bounds are not effective object-panner bounds. RPAN validator0x18000ba8c permits up to100 items in each of four groups, but initialization0x180010f2c rejects summed position count greater than32. This is an important demonstrated distinction: a structurally acceptable larger carrier can fail native initialization.

Float validators0x18000b8b8/0x18000b86c reject NaN, infinity and nonzero subnormal values. They inspect FIRs, additional response-table floats and coordinate triplets. The read-only inspector now covers these values, exact selected extents and RPAN dimensions. Its16-MiB carrier and65536-sample bounds remain conservative tool limits. It reports the32-position initialization boundary without claiming complete native acceptance. Ten project tests pass on Python3.8, including both actual carriers, nonfinite/subnormal arrays and malformed dimensions.

Response accessor0x180019008 selects each serialized FIR at `table+96+filterIndex*responseLength*4`. Constructor0x1800192e8 creates2*positionCount convolution states from these FIR pointers and native FFT setup. This is evidence of FIR consumption, beyond mere format parsing. Ear routing and complete output mixing still need their processing caller.

0x180018090 copies the floats following the FIR arrays into two sets of two per-position vectors. These92 current values include unequal left/right-like weights, zeros and gains at several levels. Their semantics are not established here; they must not be treated as an ITD vector or normalized independently. They may preserve a direct mixing path alongside FIR convolution. Native panner state, these vectors and device-profile processing must all be followed before claiming that a custom FIR fully replaces the listener response.

## Evidence and next steps

Complete native code and matching snapshots are CapxSettings evidence and DtsHrtfEnc evidence. Key records:

- Property-store activation0x180001400: `ev_e8ebf31bdffd63a2db8bc55561fdcdf3ce58ec98516a20faa0461deaa6b853c1`.
- Device writer0x180001a20: `ev_fa1923e06e690b57d239557ac452379bbf5826d54cb3769f53aed6fb5f2caf0b`.
- Runtime writer0x180001c10: `ev_295694e6f185ac7fe1bf690d017094c0bae5ddb2e2ae50e2ce3f65ea7261f773`.
- RCHN validation: `ev_ad23311a5bfe7c0eb53f68f3ad13a3e96b76678512ef2c8b58f087379a5ce98d`.
- RPAN validation: `ev_46434ec48f6521dae2241d72e1415be943e9cd790da6f51f994934edb82e7071`.
- Panner32-position limit: `ev_d953d192bd5cfe16b657a7cb339178ac302e0f58b59d3f218deb3205acd63d65`.
- FIR pointer accessor: `ev_cdd86966811f380d2c1688c3625838c6ba5b590c5d3d78a42e3de043cb705ce5`.
- Convolution-state construction: `ev_be3f7fde931c351703850b8ea7e04f60a45e400df8dbc50b21d0a5ebd01dee4f`.
- Additional-vector copy: `ev_6fcb4d7319504b5e7d633ae9ac89cd76dadd9241b7686836692775827e0ce660`.

Next: connect these endpoint keys to the encoder's RuntimeSettingsBLOB/DeviceProfileSettingsBLOB bag, resolve object coordinates/group/ear/mixing semantics and notification/cache behavior, then produce a backed-up one-endpoint import proof. Any23/32-position conversion will use the native panner's representation and must quantify held-out directional error; it cannot claim to install710 independent native FIR directions. No settings writer or SOFA converter was executed or supplied as working by this pass.
