> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Near-anechoic runtime controls

The retained normal DAHP Runtime PID2 message does **not** contain the advanced speaker-virtualizer control group. Its presence byte is API357 / registry365 and is zero. Near-anechoic is a Boolean after native normalization, but its wire representation is the fifth signed32-bit integer in an optional eight-word group. There is no existing near-anechoic byte to flip in this message.

Full native decoder/setter/consumer functions, offline field offsets, inspector, tests and test results are retained. The input is the exact369-byte retained registry snapshot, SHA-256 `<private-artifact-digest>`, with361-byte API payload after its eight-byte envelope. This pass did not reread or write live settings.

## Serialization and normalization

Runtime decoder0x18014e750 calls pair helper0x180150830 for decoded+0xd34, then group helper0x1801508b0 for decoded+0xd40. The latter consumes one presence byte and, if nonzero, exactly eight consecutive little-endian32-bit words; no count or alignment padding is serialized. It records presence atdecoded+0xd40 and words at+0xd44..+0xd60. Truncation marks decoder failure. Native accepts any nonzero presence; the offline inspector requires canonical0/1.

| Group word | Wire offset relative to presence | Decoded value | Requested/current core offset | Setter behavior |
|---|---|---|---|---|
| steerer_source_distance |+1 |0xd44 |0xab0 /0xab4 | Signed clamp4..256 |
| far_scale |+5 |0xd48 |0xab8 /0xabc | Initial clamp820..32767, then product bound below |
| mid_scale |+9 |0xd4c |0xac0 /0xac4 | Same |
| near_scale |+13 |0xd50 |0xac8 /0xacc | Same |
| b_near_is_anechoic |+17 |0xd54 |0xad0 /0xad4 | Any nonzero integer→1; zero→0 |
| lr_render_mode |+21 |0xd58 |0xad8 /0xadc | Unsigned0..3; larger values, including negative signed inputs, become3 |
| center_render_mode |+25 |0xd5c |0xae0 /0xae4 | Same |
| remaining_render_mode |+29 |0xd60 |0xae8 /0xaec | Same |

Runtime applier0x18011cd60 tests presence+0xd40, copies all eight words and calls setter0x1800071b0. No per-word presence exists. The setter adjusts scales toward the bound4..256 on `scale/32768 * clamped_distance`, then rounds and saturates to signed16-bit. Quantization/saturation means this is not a strict guarantee of the final floating product lying inside that interval. Constants at0x1802e87d0/d4/d8/dc are exactly1/32768,4,32768,256. Physical units are not assigned here. It sets dirty flagscore+0xaf0/+0xaf4/+0x13b0.0x1800074c0 copies requested words into current+4 partners during core application.

Null setter input and initializer0x180007530 use `[103,32568,6698,5090,1,1,1,1]`. Because the serialized group is absent, this message does not request those values or prove the live core retains them. Prior initialization, another runtime message or other state can determine effective current words.

## Exact retained offsets

The supported offline decoder walks the native field order, including the present20-pair array atAPI88, through all68 optional fields and ends exactly atAPI361. It refuses present composite fields outside the inspected prefix subset; it is not a universal runtime schema decoder.

The group is the fourth-last optional field: API357 absent; following decoded fields+0xd64/+0xd6c/+0xd7c are absent atAPI358/359/360. Output mode remains present9 atAPI0..4. The surrounding fields and values are individually recorded in the inspection JSON, rather than locating the group by searching for default integers or counting trailing zero bytes.

An in-memory parser fixture supplies the group and verifies its33-byte extent, exact word order, unchanged preceding fields/output mode9 and shifted final three presences. It is not exported as a candidate. Such insertion would expand API361→393 and registry369→401. The near-anechoic word would then begin atAPI374 / registry382. The original group has no serialized word values to report.

## Native enable predicates and candidate implication

In the identified propagation branch of core0x180007d10, `local_600` is true only when input count>=2, current output modecore+0x11c==15 and core+0x28!=0. When core+0xaf0 is dirty, that branch iterates five states atcore+0x1818+i*8, sends current near-anechoiccore+0xad4 to0x18001b4d0, and sends distance/scales to0x180018ad0.0x18001b4d0 stores the control atdirect-state+0x6c. The grouped source-index records consume current LR/center/remaining modes; no numeric mode names or Windows-category correspondence is inferred.

The previously traced0x180018b70 uses direct-state+0x6c (or PCA+0x15c when negative) when testing its room-enable result. Its counter/other predicates still apply. That is an evidenced control consumer, but the identified runtime propagation branch is mode15, whereas this stored PID2 requests9. No selected direct xref shows that the same group update is propagated into the tested mode9 object state. Disabling this field therefore has no established effect on that stream.

A one-Boolean candidate is not justified for the retained message: making the absent group present would also submit seven controls whose live effective values are unknown. Output mode9 can remain byte-identical, and PCA is in separate InitTime PID1, so neither must change structurally; that does not make the new eight-word submission an isolated Boolean experiment. If an already-present group with known effective words existed, changing only its canonical near flag1→0 could preserve the other seven. That condition is not met here. No candidate blob or live writer was generated.

Three focused test groups pass onPython3.8, covering retained boundaries, the in-memory eight-word fixture, truncation/trailing data, noncanonical flags, unsupported prefix composites, native array bounds and CLI parity. Both scripts compile. Private headless Ghidra completed; complete selected output contains no decompile failure and original consumer binary remains pinned. The exact remaining gap is effective seven-word state plus a mode9 source-profile/near-control propagation edge. The null tap result and reported directional-field gain test do not fill that gap or establish room-path uptake.
