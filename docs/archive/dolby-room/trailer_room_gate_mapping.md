> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Persisted room-control trailer

The owned v2 lane15689 is the signed integer fallback at decoded PCA+0x15c. A proposed1→0 edit, with lane1 checksum1685→1684, can isolate this persisted control while preserving all directional and room coefficient arrays. It tests both the final room-enable predicate and two upstream geometry/control branches. Static evidence does not establish the current stream's effective override, descriptor category, counter state, or native acceptance of an edited payload. No candidate or live writer was generated.

## Exact mapping

Offsets are zero-based, ranges half-open. Read-only inspection verifies the owned source and retained API/registry payload match.

| Coefficient lane | Value | Decoded PCA byte offset | API bytes | Registry bytes |
| --- | --- | --- | --- | --- |
|15688|0|0x160|62767..62771|62775..62779|
|15689|1|0x15c|62771..62775|62779..62783|
|15690|1|0x164|62775..62779|62783..62787|

The observed table-B end is15624. Four header words, four13-word distance records, and eight tail words advance the native packed-word cursor to15688. Decoder0x18010fd30 calls0x18010e5d0 once and0x18010e330 three times; each advances26bytes/13words. Version2 additionally calls0x18010e840, advancing16bytes/eightwords. At0x18011148d the first final signed16 word is stored at PCA+0x160; at0x1801114b0 the second is stored at+0x15c; the third is stored at+0x164. The original API stores each coefficient in an int32 lane; native coefficient serialization consumes its low16 bits. The header count/layout and public parser cursor independently agree with this placement. Complete functions are in decoder and wrapper evidence and cursor helper evidence.

There is no Boolean normalization, clamp, or enum bound on these three words in the selected fast decoder: it sign-extends each low16 word. For versions other than2, all three fields are explicitly zeroed at0x180111457..0x180111463. This is a format fallback, not proof of a live default. The generic fallback decoder0x18010cb30 also writes0x15c, but it follows failure of0x18010fd30; it is not the observed successful v2 decode path.

## Consumers and overrides

Instruction evidence resolves the decompiler's misleading float type for the control: native code uses integer signed and zero tests.

*0x180018b70 reads direct-state+0x6c at0x180018b73. If negative,0x180018b7c reads PCA+0x15c. If state+0x14!=0 and state+0x18==0, the unsigned counter+0x1c increments while below PCA+0xc. Once at/above that threshold,0x180018ba4..0x180018baa returns `incoming_enable & (effective_control==0)`. If state+0x18!=0 it resets the counter. Other cases preserve incoming_enable. Therefore changing fallback1→0 can preserve an already-enabled room branch after the threshold; it does not force enable past other conditions.

*0x18003e550 independently reads the same state override at0x18003ea3f, falls back to PCA+0x15c at0x18003ea56, and retains the integer at stack+0x300. When source descriptor word+0x28 equals1, control!=0 has two effects: at0x180041263 it sets the per-source intermediate record+0x18 to1; at0x180041223 it zeros a scalar used by the subsequent four-value output gain computation. With control0 those branches use the ordinary computed scalar. The complete decompilation is in consumer evidence, function0x18003e550, especially lines2622 and2998. This proves additional control consumption; it does not justify claiming unchanged direct transfer merely because PCA coefficients are unchanged. The numerical meaning/name of descriptor category1 is not established here.

*0x180025a80 passes the selected PCA pointer from wrapper-pair+8 to both functions. It runs geometry first, calls0x180018b70 for the current enable, runs direct/room-send0x1800420d0, and invokes additive room processor0x18001b700 only when the returned enable is nonzero. The selected room's+0x7c band-count test supplies the wrapper enable. Thus this trailer can govern the gate even while the room coefficient table itself is unchanged.

*Constructor0x18001b4f0 initializes direct-state+0x6c to-1, making fallback possible. Setter0x18001b4d0 can override it; the previously identified runtime propagation branch is mode15 with DSO active. Neither constructor defaults nor the absent Runtime PID2 near-control group proves the tested mode9 stream still has-1.

The whole-image immediate0x15c instruction scan also found0x180018bc0 and0x180045630. Inspection shows those accesses belong to intermediate transfer/filter storage, not a PCA pointer read; indexed state+0x15c in0x18003e550 likewise belongs to per-source records. These are not additional established consumers of this PCA member. The scan cannot exclude aliasing, vector copies, or computed offsets; the established operational consumers are the two above, not a proof of exhaustive whole-program data flow.

## Proposed isolated diagnostic and limits

Only allowed coefficient edits would be lane15689 and checksum lane1. Corresponding API ranges are[62771,62775) and[19,23); registry ranges[62779,62783) and[27,31). XOR checksum changes by1, from1685 to1684. Values0 and1 are exactly representable by the native signed16 decoder. Preserve lane13 exponent, room lanes14..1833, directional lanes1834..15623, every other trailer lane, envelope, version, initialization fields, runtime mode9, endpoint and category. Native validation/uptake must still be checked in a future explicitly authorized test; the checksum identity alone cannot prove them.

Use current personalized94/API and04/registry as baseline and exact restore, with identical object positions, category, sample rate, level, sweep and tail. Compare paired current/control0/current recordings, including direct and early/late energy, without normalization that hides gain changes. A null result remains compatible with a nonnegative source override, descriptor category!=1, inactive/counter-inapplicable gate, different selected room, or rejected/unused modified trailer; it cannot by itself distinguish those conditions. No toggles or Access launch are justified by this static result.

The read-only inspector was run under Python3.8. Three tests pass: exact owned mapping/checksum arithmetic, truncated input rejection, and mismatched source rejection. A second pass checked signed-vs-float decompiler typing against assembly, cursor lengths against helper functions, and source/API/registry SHA256 identities. No coefficient mutation, candidate serialization, DLL execution, endpoint/property read/write, or playback occurred in this pass.
