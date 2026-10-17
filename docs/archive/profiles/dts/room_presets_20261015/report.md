> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# DTS personalized FIRs and native room response — 2026-10-15

The SOFA converter muted the native room input. Restoring the first stereo auxiliary vector set restores a measurable recursive room response while retaining every personalized FIR coefficient byte. Balanced and Spacious are now mapped through the app's shared preset objects, localized resources and asset-filename initialization. Both room-send variants have been tested through native Windows spatial-object rendering and paired virtual-endpoint recording with identical personalized FIRs.

## Verified cause and correction

Ghidra analysis of the pinned DtsHrtfEnc.dll identifies initializer `0x180018090`, mixer `0x18001846c`, room constructor `0x18001688c`, parameter initializer `0x180016b58`, and recursive processor `0x18001764c`. The mixer accumulates each node through its left/right auxiliary sends, processes that stereo bus through filters and recursive delay/feedback branches, then adds the result to the independent FIR outputs.

Earlier reports described this first auxiliary set as a dry/GPEQ return. That interpretation was incorrect. The downstream recursive room processor establishes its room-input function. The second auxiliary set remains unresolved and is left zero in both personalized candidates.

The new candidate changes only the first auxiliary set at bytes `[753796,753980)` of the 754,556-byte personalized carrier. Ninety-six bytes differ. It preserves the personalized FIR hash `<private-artifact-digest>`, room parameters, panner, and second auxiliary set. Candidate SHA256: `<private-artifact-digest>`.

`../../tools/convert_sofa_dts.py` now accepts `--room preserve` or `--room off`. The existing default remains off. Preserve copies the native room sends beside the converted personalized filters; it does not replace those filters with stock filters. Both modes reproduce the existing native-tested candidate bytes exactly with the accepted 4,096-sample SOFA; see `converter_parity.json`. An optional `--room-template FILE` supplies a different native room-send set while retaining the original template's personalized FIR conversion, origin response, header and panner. It requires matching native sample rate, table mode, panner and room parameters and refuses incompatible templates. It exactly reproduces the native-tested Spacious candidate; see `sd2_converter_parity.json`.

## Native test

`native_room_send_test/result.json` and `analysis.json` record stock, personalized room off, personalized room preserved, and stock restored. Each state contains two repetitions at 30 degrees left, 90 degrees left, and rear, rendered as dynamic spatial objects through DTS on selected virtual endpoint. No per-ear normalization was applied.

- First 5 ms energy change from restoring the sends: −0.000540 to +0.000027 dB.
- 5–20 ms energy: +3.746 to +4.540 dB.
- 20–250 ms energy: +14.251 to +15.227 dB.
- Full compared-response relative L2 difference: 0.055010.
- Stock versus restored relative L2 difference: 0.000370.

The personalized FIR bytes are identical; the early response remains effectively unchanged and the later response increases. This supports restoration of a parallel native room return. Preset identity is established by the app/resource mapping below; listening quality has not been evaluated.

Fresh backups were taken. The original runtime and transaction counter were restored exactly, configuration comparison has no differences, global defaults were unchanged, and cleanup reports no errors. Dolby data was unchanged. The test did not leave the personalized candidate installed.

## Balanced and Spacious investigation

Installed DTS Sound Unbound 2026.3.31.0 contains encrypted Gaming Balanced, Gaming Spacious, Gaming Spacious2, Movies Balanced, Movies Spacious and Neutral assets. Ghidra function `0x180920690` constructs six option records from those filenames. The selected UI presently reads Spacious and Generic Over-Ear Headphones, but the application may target the physical default endpoint. No preset was clicked and no app settings were changed.

The app's private native async methods lead from runtime parameter selection through cache/file loading and a transformation before the carrier reaches the native property writer. These methods are absent from the package WinMD public interfaces. Targeted decompilation is saved in `app_targeted_verified.txt`, `app_blob_readers.txt`, `app_blob_helpers.txt`, `app_blob_actions.txt`, `app_blob_decode.txt`, `app_preset_selection.txt`, `app_preset_index.txt`, and `app_preset_filename.txt`. Function ranges are verified against PE exception entries; indirect .NET Native targets and separate exception funclets remain limitations.

Important distinction: the six-value switch in `0x180b1d1e0` returns spatial-provider GUIDs, including the DTS Headphone:X provider at index zero. It is not a Balanced/Spacious index switch. Similarly, encoder table mask bits 1/2 do not establish preset names.

A read-only scan of the already-running app's private heap recovered three structurally valid SPAC carriers. Only complete validated SPAC carriers were saved; no full process dump was written. Their asset identities were established subsequently by offline decryption, as described below. See `readonly_app_heap_spac/comparison.json` and its acquisition manifest.

### Offline preset decoding

The app constructor guard at RVA `0x78fa20` points to constructor `0x9228b0`, which initializes the static asset-decoding material. Functions `0xb39830` and `0xb383a0` configure the transform and copy a length-prefixed carrier from its result. Offline AES-128 ECB with PKCS7 padding reproduces the packaged transformation. Every one of the nine copied assets passes decrypt/encrypt round-trip equality and structural SPAC validation. The six packaged assets contain 32 trailing decoded bytes beyond the length-prefixed carrier; the app's copy length excludes these. No vendor application, account, license or endpoint state was changed to decode them. See `decode_packaged_presets.ps1`, `decoded_presets_and_cached/manifest.json`, `decoded_packaged_presets/comparison.json` and `decoded_roundtrip_validation/manifest.json`.

The named Gaming Balanced and Gaming Spacious assets use the same ten-node panner and the same serialized recursive-room parameters. Their nonzero room sends differ by approximately 10 dB. Neutral has zero room sends. Spacious2 raises them another approximately 5 dB over Spacious. Movies variants additionally change the front-center room send. These named presets also contain differing stock FIRs; importing their entire carrier would replace personalization. A compatible room-only operation must isolate room fields.

The cached SD1 headphone asset decodes exactly to the original endpoint runtime hash `967db8...`. SD2 decodes to `42c3a4...`. The speaker SD0 asset decodes to `eefdd5...`. These establish the source of all three recovered heap carriers. SD1 and SD2 have the same 23-node panner and recursive-room parameters. Their room-send gains differ by 10 dB at most nodes, with a 9 dB difference at one rear pair. Their second auxiliary sets match.

The generic UI mapping is established by object identity, rather than by matching coefficients:

1. Constructor `0x18091fe40` creates the two shared preset objects at global slots `0x180782858` and `0x180782860`, with values0 and1.
2. Label-map initializer `0x180b4e220` associates those same objects with resource keys `configure_sound_combobox_natural` and `configure_sound_combobox_spacious` respectively.
3. The installed PRI resource dump gives the English values **Balanced** and **Spacious** for those keys. See `resources_dump.xml` lines1572–1595.
4. File-map initializer `0x180b47320`, provider branch0 (DTS Headphone:X), associates the first object with the SD1 filename triple and the second with SD2.
5. Filename constructor `0x180922980` initializes the Windows headphone names to `02-SPAC-HqHeightAndHgNf_SD1_Hp_Normal_v4_RC2.SPAC` and `04-SPAC-HqHeightAndHgNf_SD2_Hp_Normal_v4_RC2.SPAC`; `.crypt` is appended by the file-map initializer. Xbox names are a separate conditional branch.

Evidence is in `app_sad_enum_init.txt`, `app_sad_option_consumer.txt`, `app_legacy_preset_mapping.txt`, `app_legacy_asset_names.txt`, and `resources_dump.xml`. The separate functions `0xb49950` and `0xb49f30` parse PluggedIn/OnBattery **power** options BestPower, BestSound and Balanced; they do not map spatial room presets. The shared word Balanced initially made that an ambiguous lead; its surrounding keys exclude it from this mapping.

### Second native comparison

`native_sd1_sd2_test/result.json` and `analysis.json` contain a second four-state, 24-object-sweep comparison using the identical personalized FIRs with SD1 and SD2 room sends. SD2 candidate SHA256 is `<private-artifact-digest>`; its FIR hash remains the same `e7f892...`. Only bytes in the first auxiliary-set interval change.

- First 5 ms energy: −0.001701 to +0.000061 dB.
- 5–20 ms energy: +7.909 to +8.306 dB.
- 20–250 ms energy: +9.819 to +9.866 dB.

All captures completed; stock runtime and exact transaction counter were restored, raw/configuration snapshots match, global defaults are unchanged and cleanup reports no errors. Thus a stronger native room can be applied while retaining personalized FIRs, without relying on the app to preserve the injected carrier.

The working preservation method is to retain the personalized carrier and copy only the first stereo auxiliary set from the mapped Balanced or Spacious template. Both templates' recursive-room parameters and second auxiliary set match, so replacing other fields is unnecessary for their room-send difference. Their stock FIRs differ; therefore this reproduces the selected preset's native room return beside the personalized direct filters, rather than reproducing its entire stock binaural response. The table's final calibration word is retained from the personalized carrier.

Use `--room preserve --room-template <decoded SD1 file>` for Balanced or the decoded SD2 file for Spacious, with the original conversion template and the same SOFA input. `candidate_room_send_preserved/runtime_SPAC.bin` is the tested personalized Balanced carrier; `candidate_sd2_room_preserved/runtime_SPAC.bin` is the tested personalized Spacious carrier. `--room off` reproduces the prior dry personalized candidate. The tested local control scripts remain hash-pinned and backup-bound. Changing the preset in the vendor app may replace the complete carrier and erase the injected personalized FIRs, so a repeatable control should apply the combined carrier and transaction notification itself.

## Lower-priority Dolby result

`../../../DolbyHeadphoneRoomResearch/tools/make_room_gain_probe.py` creates a room-output-tap gain candidate at 1.5 times the enabled-room reference. Three tests cover unity low-word parity, isolated edits, and rejection of invalid gains, changed input, and signed-Q15 overflow. The pinned native Dolby size validator/deserializer accepts it with intact scratch canaries. Decoded directional fields, feedback and mixing matrix hashes are unchanged. See `DolbyHeadphoneRoomResearch/candidates/room_output_gain_150_v1/manifest.json` from the workspace root.

The predicted room-return increase is 3.522 dB. This is output-tap scaling relative to the captured enabled-room coefficients, not a calibrated Dolby UI percentage. It has not been installed or recorded through the live renderer. Two times tap gain overflows the existing signed-Q15 coefficients and is rejected rather than clipped. Increasing room level and changing decay require separate controls: feedback modifies decay, while output-tap gain leaves the recursive feedback coefficients unchanged. Further improvement requires measured rendering and listening, not parser acceptance alone.

## Validation

Nine converter tests pass, including matching personalized FIRs across room modes, copied room-send bytes, untouched second vectors, separate room-template selection and rejection of incompatible parameters and unsupported room labels. The fixture-specific restoration helper's two tests and its native parser checks passed. Ten in-memory write/restore transaction tests passed for each copied writer before its live experiment. Converter parity with all three native-tested personalized carriers was rechecked after implementation. The wider capture-getter tests originally assumed Dolby was selected; they now inspect which of the two supported providers is actually selected, verify its expected profile, and verify rejection of the other provider without starting a stream. This keeps the test read-only and independent of the saved provider selection.

The complete spatial research test suite passes35 tests. The Dolby room-gain helper passes3 tests. Native build uses warnings as errors. Both bounded live experiments restored their saved runtime/counter/configuration; no room candidate remains installed from these experiments.
