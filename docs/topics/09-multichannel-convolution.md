# Object rendering to sixteen-channel convolution

## Explicit speaker geometry

Given available object PCM, position and gain, vector-base amplitude panning solves for nonnegative gains on a surrounding speaker triplet. Normalize squared gain consistently and apply source volume once. A valid three-dimensional layout surrounds the listener; an uncovered direction is an error, not a reason to invent an extra signal.

The native panner supports explicit spherical speaker coordinates and 4–32 output channels in the tested implementation. Tests include 64 sources into sixteen buses. Pan weights and source gain transition across a declared producer quantum. If weights are renormalized during interpolation, that policy should be part of the reference; an accurate final direction does not prove constant angular speed during the transition.

## Object lifetime versus filter state

The v2 carrier includes explicit uint64 object IDs. A new lifetime resets that slot's previous pan/gain values. It does not reset the speaker-bus convolvers, whose histories still contain earlier audio. This separation preserves the earlier object's full response tail while the new object begins at its own location/gain.

Source IDs, generations, update ordinals, exact integer sample offsets and EOS transitions are validated. Review exposed a Python negative-offset bug: a matched malformed capture/reference pair could select PCM from the end of the array. The corrected converter requires contiguous extents and rejects malformed offsets before rendering.

## Hosting the actual APO DSP

The isolated host obtains the installed Equalizer APO class through DllGetClassObject, uses its APO/configuration/RT interfaces and supplies its own configuration. Only the privately loaded image's configuration-read imports are redirected. No installed master configuration is written or replaced.

The sixteen-bus binaural matrix duplicates inputs into 32 temporary convolution channels, applies one ear filter per route, sums into output columns 1/2 and zeros columns 3–16. Actual DLL format negotiation rejected a sixteen-input/two-output graph. Keeping sixteen physical columns with two active columns is a verified but different arrangement. A live host must explicitly route the intended columns downstream.

Actual captured Windows objects passed the complete panner→APO chain against an independent float64 panner/convolution reference, with all 4096 taps and final tails. The custom-rendering stage was offline. This does not establish a live endpoint route or equivalence to per-object filtering at every possible direction.

## Fixed processing quantum

The tested APO convolver expects a fixed quantum. Alternating callback lengths produced wrong samples despite intact external guards. Source review corroborated partition preparation tied to callback size, including a first-call reinitialization path. Coalesce producer packets into fixed 128-frame processing and maintain a separate output FIFO.

The host's silent-buffer policy also matters: BUFFER_SILENT suppressed a tail under the tested policy, while valid zero PCM retained it. Another installed policy can preserve tails. This is not a universal statement that every Equalizer APO installation truncates silence.

## Why sixteen buses

Bus convolution cost is tied primarily to the chosen speaker count, which can be useful for many objects. The tradeoff is directional approximation: panning to sixteen fixed filters is not identical to directly applying a distinct SOFA response to every source direction. Both architectures are useful, but they preserve different information.

The live integration still needs an input observer, fixed-quantum adapter, bounded output FIFO, device callbacks, source/output clock policy, discontinuity handling and native-output suppression. The ordinary post-mix APO cannot supply original coordinates by itself.
