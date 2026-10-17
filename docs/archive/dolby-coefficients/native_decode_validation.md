> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Native coefficient parser validation

The primary agent executed the pinned consumer's size validator and rp deserializer in isolated child processes against three reviewed fixtures. Their native-decoded directional arrays match this project's independent logical-field inspector exactly. This advances the evidence from static/public-parser agreement to actual native coefficient deserialization. It does not establish live renderer uptake, synthesis, physical ear routing or HRIR recovery.

This documentation pass inspected the harness, saved results and retained native decoder map. It performed only an offline JSON consistency check; it did not load or call the native DLL, rerun the harness, conduct further static analysis or change live settings.

## Reviewed execution boundary

The primary harness admits an x64 process, an exact62,791-byte API fixture and one of three pinned fixture SHA-256 values. It checks the complete consumer hash `0fada40251880d03c657a357745d362799c93fb6024f733f1c8bf1057f5e4715`, decodes the API envelope with the strict existing inspector, and narrows all15,691 entries to low16 words before invoking the two native functions:

- RVA0x10dab0: size/checksum validator with the low16 rp array.
- RVA0x10fd30: fast rp deserializer with sample-rate48000 and caller-owned bounded scratch memory.

The DLL is loaded by the harness, so this was native library execution including normal loader initialization. Its selected explicit entrypoint calls are parser functions. The inspected source contains no COM activation, audio endpoint, renderer/stream creation, playback/capture, registry/property setter or configuration mutation. Child-process isolation was arranged by the primary; the script itself is the child entrypoint and does not spawn another child.

Scratch allocation must be nonzero and at most0x21380. The harness aligns the usable address to32 bytes, surrounds the requested extent with unchanged0xcd canaries, and rejects returned pointers whose requested span falls outside that extent. All inspected float arrays must be finite. Input API bytes and the DLL hash are reread after decoding. These checks bound the inspected accesses; they are not a general security proof for arbitrary malformed inputs, which the fixture allowlist excludes.

## Results

| Fixture | API SHA-256 | Size/decode status | Scratch | Directional logical max error | Room taps nonzero |
|---|---|---|---|---|---|
| Current personalized baseline |<private-artifact-digest> |0/0 |67,032 bytes |0/0 |256 |
| Room output taps zero |<private-artifact-digest> |0/0 |67,032 bytes |0/0 |0 |
| Both directional fields half gain |<private-artifact-digest> |0/0 |67,032 bytes |0/0 |256 |

All three results report unchanged scratch canaries, native decoder version output2 and secondary output word2. Native version output2 is distinct from the source coefficient-version string`1.1.0`. The secondary word is named`decoded_extra` by the harness; its complete semantic meaning is not established here, and it is not the owned Table-A extra-path count0.

Saved primary results: baseline, zero room taps, half directional gain.

## Native directional order and scaling

The inspected harness reads PCA pointers at byte+0x110/+0x118 and80 x36 x2 float32 values from each. It transforms the native SIMD storage as:

```python
actual = values.reshape(20, 36, 2, 4).transpose(0, 3, 1, 2).reshape(80, 36, 2)[:77]
expected = logical_field(serialized_lanes, 2)
max_error = np.max(np.abs(actual - expected))
```

Each full native field is5,760 floats/23,040 bytes, including three padding bands. The first77 bands are5,544 logical real/imaginary values. This agrees with the retained native`0x18010e9f0` field decoder, which reads154 x36 signedQ15 lanes, uses shared exponent2, writes its four-band SIMD index and zero-pads to160 x36 floats. The first field consumes source lanes4500..10043 and the second10044..15587. There is no invented direction grid, phase, normalization or synthesis in this comparison.

All three native results report maximum component error exactly0 for both fields. This validates the independent low16 sign, shared-exponent scaling and first77-band real/imaginary order for these fixtures against the actual decoder. Padding participates in the full-native-array hashes but is excluded from the reported logical parity comparison. The hash equality below establishes that padding and storage bytes are also unchanged for the room-only candidate.

The baseline and zero-room-tap candidate have exactly the same native field hashes:

- Set0: `<private-artifact-digest>`.
- Set1: `<private-artifact-digest>`.

Each has3,492 nonzero native field values. The half-gain candidate instead has hashes`<private-artifact-digest>` and`<private-artifact-digest>`, with3,490/3,489 nonzero values. Its exact-zero parity is against the altered, quantized source lanes, not against an unquantized ideal multiplication by0.5. The candidate manifest specifies nearest-integer ties-to-even rounding and maximum decoded-coefficient rounding error6.103515625e−5. Loss of a few small nonzero values is consistent with that quantization.

The room taps are read through room+0x50 as320 float32 values, including the padded fifth64-band branch. Baseline and half-gain candidate share native tap hash`<private-artifact-digest>`; the zero-tap candidate has hash`<private-artifact-digest>` and no nonzero values. Thus native decoding preserves the tap block for the directional-only diagnostic and preserves both directional blocks for the room-tap diagnostic.

## Terminology and remaining boundary

No substantive dimensional, sign, scaling or order mismatch was found. Two result labels need conservative interpretation: `room_enable_at_7c=64` is the room band's dimension/count field, not a Boolean enable value; and `decoded_extra=2` is the decoder's secondary output word, not the Table-A extra-path count. These distinctions were reported to the primary rather than silently changing its result files.

The saved native statuses and arrays demonstrate parser acceptance and correct decoded data for the admitted fixtures. They do not establish selection of those pointers in a live audio stream, amount of room contribution, native half-gain output behavior, persistence/UI installation, aggregate normalization, final physical ear assignment, original acoustic HRTFs or a valid recovered SOFA. Those questions remain separate from this native deserialization validation.
