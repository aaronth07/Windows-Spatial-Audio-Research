> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Consumer Dolby headphone room parameters

This map applies to the owned 15,691-lane rp payload, version 1.1.0, and the inert consumer DolbyAudioProcessing.dll SHA-256 `0fada40251880d03c657a357745d362799c93fb6024f733f1c8bf1057f5e4715`. It separates serialized lanes, decoded native pointer fields, and modeled effects. It does not treat a valid container as proof of native uptake.

2026-10-14 update: native uptake and room control are now verified. Lane15689 at PCA+0x15c is the fallback room-send/gate suppression control;1 is the current short-tail setting and0 enables the tested room return. With0 fixed, zero taps and half feedback change native output while preserving the direct transfer within repeatability. The controls also work within an existing object stream. Verified results. The proposal/unperformed wording later in this original offline map describes its earlier evidence stage.

All coefficient ranges below are zero-based and half-open. The signed-int32 coefficient array begins at API byte 15, registry byte 23; native DecodeMessageTuning converts every entry to its low 16 bits before decoding. API bytes for lane k are `[15+4k,19+4k)`. Native rp bytes for a lane are `[2k,2k+2)`.

| Coefficient lanes | Function | Encoding and native consumer | Room-only edit status |
|---|---|---|---|
| 0 | rp magic | 0x7072; native 0x18010fd30/0x18010dab0 | Preserve |
| 1 | Global checksum | XOR seed 0xa569 over lanes 2..15690; owned extended checksum 1685 | Repair for any coefficient change |
| 2..12 | Presence, dimensions, counts, controls | Owned words `[1,1,0,64,3,0,1,25,20,11,36]`; size/offset/allocation formulas depend on these | Preserve; not room gains |
| 13 | Shared field exponent/control | Owned 2. Public directional complex fields use this exponent; native PCA decoder also receives exponent information | Preserve; clearing table A wholesale is not a room-only change |
| 14..16 | Allpass delay IDs | `[1,2,3]`; native room +0x08 pointer; state/allocation and integrity depend on IDs | Preserve for output attenuation |
| 17..19 | Allpass gains | Q15 `[.75,.75,.75]`; room +0x10 pointer | Changes room recursion; less minimal than output attenuation |
| 20 | Room branch scalar | Q15 0. Native 0x180006750, initializer, and 0x18001bdf0 select branches at 0.5 | Algorithm selector, not a demonstrated master gain |
| 21..1044 | Room ear output filters | 16x64 Q15 lanes padded/reordered to 20x64 native floats; room pointer +0x20; consumed by 0x18001b960 | Room-specific, but 1024 lanes and larger than tap edit |
| 1045..1048 | Four delay integers | `[25,23,17,8]`; room pointer +0x30 | Keep nonzero/valid; recursion timing |
| 1049 | Fifth room delay integer | Owned 7; native room +0x38 scalar. Public model uses it as tap delay; complete native consumer remains partly traced | Keep; not a demonstrated output gain |
| 1050..1561 | Complex feedback filters | 8x64 Q15 padded/reordered; native room +0x40 pointer; recursive feedback path | Changes decay/stability; avoid first |
| 1562..1577 | 4x4 mixing matrix | Q15; room +0x48 pointer | Changes feedback mixing; avoid first |
| 1578..1833 | Four-branch output taps | 4x64 Q15 padded to 5x64; decoded room +0x50 pointer; 0x18001b960 multiplies delayed branch state by these before ear filtering and adds stereo output | Smallest complete coefficient-block candidate for attenuating this room output stage |
| 1834..15623 | Directional PCA table B | Sample-rate/exponent, 36x36 transforms, two 77x36 complex fields, phase flags/increments, model scalars | Preserve byte-for-byte |
| 15624..15627 | Shared header scalars/control trailer | Native PCA/header struct +0/+4/+8/+0xc; public distance/direct evaluator consumes these | Do not classify as exclusively room parameters |
| 15628..15679 | Four distance profiles | 4x13 mixed Q15/control/exponent fields; public geometry/direct and room-send calculation share them | Do not use as first room-only edit |
| 15680..15687 | Profile values | Q15; public evaluator uses profile weights/scalars | Shared effects not fully mapped; preserve |
| 15688 | Post field +0x160 | Owned0; signedlow16; other selected consumers not fully mapped | Preserve |
| 15689 | Room-send/gate suppression fallback +0x15c | Owned1; signedlow16;0x18003e550 room-send flags/gains and0x180018b70 final enable predicate | Native-tested1→0 enables room; small shared direct normalization change |
| 15690 | Post field +0x164 | Owned1; signedlow16; other selected consumers not fully mapped | Preserve |

The owned table has A-extra count 0. Extra paths in native `0x18001bb80` use room +0x5c count, +0x60 delay list, +0x68 complex coefficients, and +0x70 mixing matrices. They are independent additions in the output computation and would invalidate a general claim that tap zeroing silences every room path for profiles with extras.

Decoded room +0x28 is the separate header lane 9 value 25, not lane 1049. The native room configuration helper reads +0x28 along with counts; these similarly named delay-related fields must not be conflated.

## Native field-to-output bridge

The selected dossiers are saved in native_room_evidence.json, including complete selected pseudocode and limitations:

- `0x18010fd30`, `ev_04602e5a1a2511e60da0436e61e0a0b60eee96e669a399000944ab22785a689a`: rp fast deserializer. After 1024 ear-filter lanes, five integers, 512 feedback lanes, and 16 matrix lanes, it converts 4x64 signed Q15 tap lanes, pads to 0x140 floats, and stores their pointer at uint32 pointer slot +0x14, byte +0x50. The reorder for source tap i is `(i & ~15) + 4*(i & 3) + ((i >> 2) & 3)`.
- `0x180006750`, `ev_d0514939fd1ab4216f30386099615ccf116fbf4892f6e7eee4204c385744af6f`: reads decoded room counts and scalar, and emits branch configuration. It does not treat scalar +0x1c as master volume.
- `0x180025a80`, `ev_39d5e00b8bbd0f396e5fa732bda017921f460f3e6fc93bcb16939f8f26ae76a1`: invokes direct rendering through 0x1800420d0, then invokes room wrapper 0x18001b700 when its room branch is selected. This establishes a separate summing stage, not identical Windows dynamic-object and shared-stereo paths.
- `0x18001b700`, `ev_9a2093c9b17013c130081efb06e8b04953bc2edbd562d2e30734a3fc7fc99ed5`: sequences room input/feedback, extra contributions, and output accumulation through 0x18001bdf0 / 0x18001bb80 / 0x18001b960.
- `0x18001b960`, `ev_9e9db6a0cbdb51620911e4a0ee0e95fc64e55b4c102ffaee69d5f6dde9726876`: computes `delayed_state * room[+0x50] + extra_state`, complex-multiplies by room ear filters at +0x20, sums branches, and adds to existing left/right buffers. There is no PCA pointer read in this selected output function.
- `0x18001bb80`, `ev_ddf9584522b0258c6cc2c88affbf93f40ef4534127d5839c8447b05fb575b631`: clears the extra-output buffer, then conditionally adds extra delayed/mixed fields. Owned extra count 0 makes this selected extra path empty.
- `0x18001bdf0`, `ev_16f4448134743e175907651692a05df1831a5a6d58c297107c6bd948d7ef8bf0`: selects alternative room processing when room scalar >=0.5. Its ordinary branch contains state/feedback processing. Table A is a recursive room filter, not only a list of early reflections.
- `0x180109ef0`, `ev_bb7dc2e110fd1b51fd30597f6daa9df75e0e01bdb22cbfa8967453a8c2a50b52`: generic tagged deserializer corroborates distinct room output, feedback, matrix, and tap destinations. The fast rp path above anchors exact owned serialization offsets.

Prior consumer initialization and runtime-control evidence remains in room_path_followup.md. It establishes separate room/PCA pointers, alternate room selection, and fixed source-distance/near-profile defaults. Actual current runtime overrides, grouped rendering-mode enum names, and all active alternative room selections remain unresolved.

## Proposed output-tap candidate

The generated offline candidate replaces all 256 output taps with zero and repairs only the global checksum. The native/public-model evidence supports suppression of this selected tap contribution; it does not prove a complete room-off mode.

Original current low-reverb personalized API: 62,791 bytes, SHA-256 `<private-artifact-digest>`. Candidate API: same length, SHA-256 `<private-artifact-digest>`.

Allowed API edit intervals are `19,23)` for checksum lane 1 and `[6327,7351)` for taps. Actual changes total 513 bytes, confined to these intervals; full half-open change ranges are in [candidate manifest. All table B, shared exponent lane 13, trailer, counts, dimensions, version, Boolean/integer controls, and registry envelope remain identical. Checksum changes from 1685 to 18519; both remain extended-valid and core-only-invalid, matching the owned extended format. JSON candidate changes the coefficient array only; JSON whitespace changes are separate from parameter changes.

Files: API payload, registry-envelope reference, and offline profile. These contain owned profile data and remain project-local.

Smaller partial-band tap edits could use fewer bytes but would leave some room output, rather than uniformly attenuating the selected output stage. No verified single serialized master room-gain field was found. Runtime near-is-anechoic is already true in default configuration and applies to a profile branch; actual runtime mapping is unknown. The current proposed tap block is the smallest complete, mapped output-stage edit found in this bounded pass, not a proof of global minimality.

## Tests and remaining obstacles

Executed offline: eight unittest groups covering pinned original/API-registry separation, byte coordinates, invalid/truncated/oversized/shadowed messages, invalid coefficient/header/checksum data, byte-level candidate isolation, output file preservation/overwrite refusal, command-line inspection/generation/error paths, and public-model output/direct isolation. The candidate profile parses; its tap array is exactly zero. A 512-slot public room unit impulse becomes exactly zero while the original is nonzero; direct gains and room-send scalar remain bitwise identical at three positions in each near/mid/far profile. These are public-model tests, not native execution.

Native acceptance is unperformed. The native parser must retain valid room and PCA pointers; a success return alone can conceal fallback. The pinned current live writer rejects arbitrary new candidate hashes, so it must not be reused unmodified for delivery. Any future narrow writer needs exact candidate/restore identities, one-property scope, backup/readback, unchanged other properties, and verified final current-profile restoration. Alternate default-room selection and additional active paths can bypass modified taps; actual path uptake still needs the matched consumer experiment.

An exact future offline prerequisite is to compare native-decoded original/candidate room structs if a supported isolated native-deserialization harness is authorized: unchanged PCA/data pointers and values, unchanged room dimensions/delays/feedback/ear filters, only room tap floats zero. No native DLL was loaded for this pass. Current container checks and public model cannot replace that test.
