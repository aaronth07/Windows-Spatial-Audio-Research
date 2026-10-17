> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Directional ear sets and consumer output routing

The bounded static pass establishes the selected native subband order and a downstream interleaved float32 PCM layout. It does not complete the intervening synthesis/alias bridge, so serialized PCA set0/set1 cannot yet be asserted as physical Windows left/right ears. No synthesis search was repeated and no binary/audio/configuration operation occurred.

The inert consumer remains SHA-256 `0fada40251880d03c657a357745d362799c93fb6024f733f1c8bf1057f5e4715`. The complete Crossfade body and a partial consumer decompiler view are in native_ear_output_functions.txt; the consumer's computed dispatch limits that decompiler view, so its raw processor call is anchored instead to the room-path agent's decoded `native_path_consumer_instructions.txt`. Earlier direct-field and block evidence remains in `native_direction_functions.txt`, `native_coefficient_functions.txt` and `native_filterbank_pivots.txt`. The independent Ghidra database was used; shared REA was not changed.

| Stage | Exact selected contract | Established order |
|---|---|---|
| Serialized fields | Lanes4500..10043 decode to PCA +0x110; lanes10044..15587 decode to PCA +0x118 | Field set0, field set1 |
| Metadata evaluator0x18003e550 | `local_1b0 = *(float**)(PCA+0x110)` and `local_1b8 = *(float**)(PCA+0x118)`; separate weighted36-term complex sums enter two per-source gain buffers | First and second field remain distinct |
| Direct accumulator0x1800420d0 | Selected complex-gain route reads source-node pointer+0x00 into `lVar40`, pointer+0x08 into `lVar8`; first gain is complex-multiplied and added to argument8, second to argument9 | Gain buffer0 to outputA; buffer1 to outputB in inspected branch |
| Wrapper0x180025a80 | Passes `pauVar39` and `pauVar22` as arguments8/9; copies outputA to `channels[0][slot]`, outputB to `channels[1][slot]`; sets channel count2 | Subband channel0/channel1 |
| Raw consumer call0x180132c7d | Module core+0x60 and scratch+0x58 passed to raw processor0x180010040; returned20-byte descriptor copied to parent+0x70/+0x80 | Output descriptor transferred; physical channel identities not encoded by that copy |
| Crossfade0x18011bd40 | Consumer passes parent+0xb0 at0x180132cc3..132cd8. Function reads descriptor channel count+0x00 and buffer-pointer array+0x18; sample addressing is `buffer0[(sample*channels)+channel]` | Interleaved float32 PCM channel order preserved by this stage |

Lane ranges above are inclusive for readability; the inspector records half-open ranges. Native structures in this table have different layouts: the subband block's channel-pointer array is at byte+0x10, while the Crossfade PCM descriptor's pointer array is at byte+0x18. They must not be treated as interchangeable descriptors.

For the selected complex-field path, metadata writes two separate gain buffers, and the accumulator adds `input*first_gain` to its first output and `input*second_gain` to its second output. The complex multiply is explicit: real output uses `in_real*gain_realâˆ’in_imag*gain_imag`; imaginary output uses `in_imag*gain_real+in_real*gain_imag`. Subsequent wrapper copies preserve the two output indices. Other accumulator branches swap paired arguments to the specialized delayed-ear helper, and metadata varies phase placement with path-difference selection. Those swaps cannot be interpreted as unconditional ear reversal. Every possible mode/branch has not been proven equivalent here.

The downstream function at0x18011bd40 is now identified by its own diagnostic strings as `dolby::oar::CrossfadeModule::Process`. It supports fade-in, fade-out and fade-out/silence/fade-in modes, computes a scalar once per sample, then multiplies every channel sample by that same scalar. Its selected data loop neither permutes channels nor mixes one channel into another. This is useful positive output-layout evidence; it does not close the missing subband-to-PCM buffer provenance.

The parent consumer's decoded instructions show the raw processor return copied into one descriptor area, followed by an optional additional call0x1800779d0 and Crossfade processing of the parent+0xb0 area. The pointer alias relationship between these descriptor areas and the accumulator's two subband outputs is not fully established. The actual synthesis dispatch/recombination chain remains unresolved as documented in native_filterbank_findings.md. The named Windows output channel-mask/PCM handoff contract is also absent from this selected evidence. Therefore it remains possible to state subband channel order and PCM interleave order separately, but not their complete correspondence.

This pass stopped after the selected caller/consumer review. It produced no HRIR/SOFA, did not invoke Dolby, and did not change the profile. Existing12 offline tests still pass and the pinned DLL/source/API hashes remain unchanged. The bounded next prerequisite is an exact synthesis/alias or Windows output bridge, or a separately authorized matched behavioral comparison handled by the primary agent. Public-parser ear labels remain provisional for physical output until that bridge is established.
