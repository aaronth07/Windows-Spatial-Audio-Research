# DTS personalized FIRs and native room presets

The tested DTS room presets can be restored while keeping the personalized FIRs. The runtime carrier also holds panner and auxiliary data.

## Runtime data is more than an HRIR table

The inspected headphone runtime carrier contains a directional FIR table, panner/origin information and auxiliary vectors used by later processing. Importing SOFA data means mapping a complete binaural field to the carrier's native direction geometry while preserving compatible non-HRIR structure. An arbitrary channel count or a file labeled SOFA is not enough.

The conversion keeps common time-domain interpolation weights for both ears, preserves relative gain and delay, and resamples only when required by the native rate. Independently aligning/normalizing ears would erase spatial cues. The template's origin response, panner and unrelated calibration data remain separate from the personalized FIRs.

## Why room disappeared

The earlier conversion zeroed two stereo auxiliary-vector sets after the FIR table. The first set was initially interpreted as a dry/GPEQ return. Focused native analysis corrected that interpretation: it sends signal into the recursive room processor. Zeroing it muted the room feed while the personalized direct FIRs still worked.

The repair copies only compatible first-set sends beside the personalized FIRs. It does not overwrite the entire personalized runtime carrier with a stock preset. The second set remains unresolved and is preserved unchanged during room-only edits. This is an example of a corrected source-level hypothesis leading to a narrower field edit.

## Balanced and Spacious

The named presets were traced through shared application preset objects, localized resource keys and filename construction to installed SD1/SD2 Windows headphone assets. The tested assets share their 23-node panner and recursive parameters; their room sends differ.

Compatibility must be checked before copying sends: sample rate, table mode, direction/node count, recursive parameters and panner must agree. Preserve the personalized FIR bytes and final calibration word. A matching file size or preset name is not sufficient compatibility evidence.

Historical native comparisons preserved identical personalized FIRs while restoring sends and recorded increased late energy with essentially unchanged earliest-window energy. The Balanced/Spacious comparison likewise changed later energy. These are measured complete-path energy-window results, not automatically calibrated RT60 or a preference judgment.

## Device correction is separate

The RDEV device configuration can embed RGPQ correction, including delays and biquad/FIR counts. The research mapped those fields and validated a correction-disabled candidate structurally/native-format-wise. That controls endpoint processing bytes; it does not synchronize the vendor application's UI selection or prove that a named EQ preset is active.

The effects context, runtime carrier, device correction and transaction counter must be treated as a complete guarded state when testing delivery. A room-only change should retain personalization. A personalization Off/On comparison should not silently compare different room-send or device-EQ states.

## Reproduction boundary

The native asset/parser code is build-specific. The public repository describes structure, compatibility and causal tests without distributing installed vendor assets, decoding material or a personal SOFA. The full archived reports retain corrected interpretations, failed candidates and the distinction between native validation and audible rendering.
