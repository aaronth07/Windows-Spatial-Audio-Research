> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Source descriptor profile1

Descriptor+0x28 is an index selecting one of four decoded distance/source profiles, not a classification of an audio buffer as direct or room-only. Profile1 is specially eligible for the persisted anechoic fallback. Its exact public enum name and the tested Windows object's current value remain unknown.

## Creation

Complete functions are retained in source producers and initializer/output consumers. Selected assembly checks the decompiler's omitted second call argument.

At0x18002c6d9..0x18002c6eb,0x18002c2b0 computes input source-record address `*(core+0x6e8)+source_index*0x3c` inRDX. At0x18002c74d it calls0x18004b8c0 with that input and a separate48-byte output descriptor inRCX.0x18004b8c0 reads input source-record+0x2c at0x18004ba6b. Values0,1,2,3 select identical output descriptor+0x28 values. The assignment for1 is0x18004bac8. Other integers cause no assignment to that output member in this switch; no clamp or named enum is established.

For standard channel descriptors,0x1800201b0 initializes descriptor+0x28 to3 and descriptor+0x2c to0. Its parameter identifies channel geometry; it is not the profile index. Caller0x180007d10's later `local_534[index*12]=1` is descriptor+0x2c, not+0x28, and must not be mistaken for selecting profile1. The previously traced mode15 runtime branch can explicitly overwrite descriptor+0x28 from grouped source-profile controls, but it does not establish the mode9 Windows stream's current index.

## Consumption

0x18003e550 selects profile storage as `PCA+0x10 + descriptor_index*0x30`. For index1 this is PCA+0x40, the second decoded profile. The selected profile supplies coordinate/distance transformation and distance-dependent gains. An additional branch can replace coordinates using per-source state; these are distinct from the PCA fallback member even when their numeric offsets coincide.

For profile1 and effective anechoic control!=0 (state+0x6c, falling back to PCA+0x15c if negative), geometry does two things:

*At0x18003edba/0x180041263 it sets intermediate record+0x18 to1. That record is linked into transfer-state+0x30.
*At0x18003f7e3/0x180041223 it zeros a scalar used to form four room-send gains. Otherwise the scalar is `distance * PCA_scalar0 * selected_profile[6]` in the retained decompilation. The four gains are stored through the per-source record+0x10 pointer.

Their room-send role is confirmed by0x1800420d0: its first loop traverses transfer-state+0x30 only when its room-enable argument13 is nonzero, skips records with+0x18!=0, multiplies each input by the four gains, and adds into argument7. Wrapper0x180025a80 supplies argument7 from0x18001b660, then passes the same buffer to additive room processor0x18001b700. Thus both the record flag and gains control excitation of that room processor.

The same intermediate source records also carry left/right directional transfer filters.0x1800420d0's later enabled direct-processing loop uses those filters independently of the initial room-send skip. Nevertheless, the scalar above enters `sqrt(1/(scalar*scalar*factor+1))` through0x1800420b0; that normalization also contributes to the directional filter construction. Therefore enabling profile1 room excitation can change direct level even with identical personalized coefficients. Recorded direct-response comparison remains necessary.

The actionable interpretation of lane15689=0 is: permit the profile1 room-send path and prevent its later anechoic room gate from suppressing an otherwise enabled room, provided the source-state override is negative and the other predicates hold. It does not select profile1 or replace room coefficients. A null A/B/A remains possible if the stream uses another profile or an overriding nonnegative control.

Validation was static: complete selected decompilations succeeded, the private Ghidra instruction export ran successfully, and a second pass checked pointer provenance,48/60-byte strides, profile offset arithmetic, and the room-input buffer handoff. No candidate, property operation, native DLL/audio execution, shared REA target change, or configuration change occurred. Remaining producer gap: the Windows-source metadata writer supplying input source-record+0x2c and its actual current value were not established in this bounded pass.
