> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Optional provider ABI and object lifecycle

Pinned AudioEng SHA-256: <private-artifact-digest>. RVA values below use its preferred base 0x180000000. This private ABI is build-specific.

Primary IID: deff1192-f581-4d77-9c1b-3e596b0ca989. No declaration was found in the searched installed SDK 26100 public headers.

| Provider slot | Inspected arguments after self |
|---|---|
| 3 Initialize | maxFrames u32, input sample rate, converted static-object mask, outputChannels u32, outputWaveMask u32, mapper pointer, diagnostics pointer, runtime pointer |
| 4 object | streamContext pointer, descriptor pointer, mono PCM pointer, frames u32, masterGain float |
| 5 metadata | streamContext pointer, descriptor pointer, metadata pointer, PCM pointer, frames u32, gain float |
| 6 bed | PCM pointer, frames u32, channels u32, gain float, flags u32 |
| 7 output | capacityBytes u32, output PCM pointer, writtenBytes pointer |

Optional IID <identifier-removed> slot 5 receives the full output UNCOMPRESSEDAUDIOFORMAT. The new wrapper's Initialize is RVA71370; object2fbb0; metadata45b00; bed2fb70; output31d20. Output discards the wrapper's frame argument and forwards byte capacity. The owning APO output channel count and mask originate from output format state, not a hard-coded stereo value.

MainPlugin Process f83b0 reads each registered stream's object count, PCM, properties and metadata before provider submission. Parser2a24c accepts 32/180-byte descriptor formats, copies bytes, clamps frames@4 to available PCM and reduces type@8 to the lowest supported set bit. Dynamic type is 1. The private MultiSource dynamic implementation f7044 compares descriptor's first u32 against per-stream instance key@0x68; -1 is unused. This establishes numeric-key use, not a generation/lifetime contract for the optional provider.

The tagged transport probe supplied its own first-u32 value 7. It proves preservation only. It does not prove that a live client may choose 7, that it is globally unique, that it survives stream reset, or that absence from one render cycle terminates the object. Flags@12 are forwarded and augmented with stream flags; the precise creation/end/reset meaning is not established here. Metadata-format dispatch also has a distinct 20-byte descriptor route.

Before connecting slot4 to persistent 16-channel VBAP or convolution state, drive the full MainPlugin path in an owned host with:

1. Two owned streams that use the same numeric object key and different PCM/positions. Observe and label stream identity from explicit registration lifetime, not pointer address alone.
2. One source spanning several packets; a short final packet; explicit EOS; a missing packet; stream Stop/Reset; release/recreate; numeric-key reuse. Record every descriptor/context byte and every provider call, plus unregister notifications.
3. Inspect descriptor-to-state update function465c4, dynamic retirement/release functions72fac/77110 and the provider's post-object/frame-completion methods. Do not guess end flags from bit values.
4. Assign an adapter-owned monotonically increasing generation only at an established creation/reset boundary. Keep retiring state and its filter tails until drained; never bind new PCM to an old tail solely because its numeric key is reused.
5. Define per-cycle output clear/accumulate/finalize, frames versus byte capacity, master gain versus descriptor gain, silent beds, invalid/unsupported metadata and short-write/error behavior. Verify independent 16-channel panning output against the parent reference with PCM/metadata snapshots intact.

This pass traced and exported the full native functions but did not run that full lifecycle experiment. The output interface is ready for a transport prototype; a stateful live renderer still depends on these unresolved lifecycle contracts and actual provider selection.
