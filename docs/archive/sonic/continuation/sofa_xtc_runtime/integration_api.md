> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Native SOFA renderer integration

Include `adapter.h`; it includes `runtime.h` and the copied `dsp.h`. The implementation requires C++17 and Windows x64/SSE2. It contains no COM or audio-device code. The system-object worker owns the COM factory/server and its source-lifetime contract.

Load these files once, outside the render call:

- `results_integration/full_sofa_banks.bin`: little-endian header `uint32 magic=0x324b4e42, bankCount, taps, sharedXtcTaps; float commonGain`, followed by float32 `[bank][tap][ear]`, then optional float32 `[sharedXtcTap][speaker][ear]`. The full-SOFA file has 710 banks, 4,096 taps, zero shared-XTC taps and common gain 1. Ear 0 is left; ear 1 is right.
- `results_integration/full_sofa_directions_windows.f32`: float32 `[710][3]` unit directions in Windows coordinates, +X right / +Y up / +Z backward. Row index matches the IR bank index.
- `full_sofa_directions_sofa.f32` provides the same rows in SOFA coordinates, +X front / +Y left / +Z up. Conversion is `windows=(-sofa_y,sofa_z,-sofa_x)`; inverse is `sofa=(-windows_z,-windows_x,windows_y)`.

Validate header dimensions, sample-rate contract (48 kHz), finite coefficients and finite unit directions before use. The prepared format has no implicit coordinate conversion or source normalization. The SOFA source timing/levels and receiver ordering are unchanged.

```cpp
runtime::BlockAdapter adapter; // Owned by one DSP instance; serialize its calls.
adapter.renderer.configure(ir.data(), 710, 4096, maxSlots, 1.f);
// configure allocates and transforms coefficients: call outside Process/Render.

runtime::Binding binding = adapter.renderer.start(fixedSourceIndex);
if (binding.slot < 0) { /* explicit capacity/duplicate-ID failure */ }

int targetBank = runtime::nearestDirection(
    windowsDirections.data(), 710, sourcePositionWindows);
// Lookup performs no allocation and searches at most 2,048 prepared directions.
// Returns -1 for an invalid/zero input vector. Directions must be prevalidated.

runtime::SourceBlock source{
    binding, pcm, 128, previousBank, targetBank,
    firstSampleGain, lastSampleGain, alphaBegin, alphaEnd
};
float stereo[256]; // 128 interleaved left/right frames.
bool valid = adapter.render(&source, 1, stereo);
```

For simultaneous sources, pass an array of SourceBlock records and its count. Keep one Binding per stable source lifetime, independently of the order of that array. The root's isolated CreateEngine test uses an explicit fixed-index lifetime until EngineRelease; that is a valid adapter contract. It must not be generalized to arbitrary game object IDs without evidence.

Each record supplies 0–128 valid contiguous mono input samples; the adapter zero-pads the rest of the 128-frame block. Start/end gain values describe its first/last valid sample and are applied once. Do not pre-apply the same gain to PCM. The renderer preserves sample time and relative source/ear levels. Apply one common output gain only when the chosen output path requires it.

That partial span describes one object's valid data inside a complete 128-frame renderer time window. If the host callback itself supplies fewer than 128 frames or another nonmultiple, stage PCM and its controls across callbacks and use an output FIFO. Do not render 128 time frames for each 64-frame callback and discard half of the output; that would change timing and lose samples/tails.

For a 1,024-frame native callback, call the renderer eight times and copy each 128-frame stereo result into the corresponding output span. Source.frameCount at offset 112 is a count, not a sample stride; source.samples at offset 104 is contiguous in the validated native source layout. Respect each source's frame count when constructing partial spans. The current adapter implements the prepared-source contract; it does not inspect or redefine undocumented native Source fields.

For a pose change over the whole 1,024-frame callback, keep A=previousBank and B=targetBank constant for all eight chunks. For chunk k:

```cpp
alphaBegin = float(128*k) / 1023.f;
alphaEnd   = float(128*k + 127) / 1023.f;
gainBegin  = callbackStartGain + (callbackEndGain-callbackStartGain)*alphaBegin;
gainEnd    = callbackStartGain + (callbackEndGain-callbackStartGain)*alphaEnd;
```

After the callback, retain targetBank as the previous bank. This matches the tested input-time model: each input sample is split between A/B filters, and all previous samples keep their original filters/tails. A Python reference can independently convolve `PCM*(1-alpha)` with A and `PCM*alpha` with B, sum the complete responses, and add exactly 128 frames of renderer delay. Interpolate native source gain across the full callback, rather than restarting the original gain ramp in every 128-frame chunk. When A equals B, the renderer uses that bank directly.

Carry the previous selected bank across callbacks. For larger movement within a callback, the source-pose interpolation and bank-selection policy must be stated explicitly in the COM test and mirrored in its reference. The tested kernel interpolates prepared coefficients; it does not claim geometric interpolation between SOFA samples. If incoming native positions are already listener-local, do not apply an additional head rotation. Control-side head-to-world quaternions are Hamilton xyzw; the inverse rotation maps world vectors into head-local vectors.

`adapter.renderer.stop(binding)` ends a source lifetime. Retired history continues until its full tail is processed. A new object can reuse that external ID in another free slot immediately; stale generation bindings are rejected. There are at most 64 slots total, including retiring histories. Capacity rejection must be handled explicitly. Reconfiguration invalidates old bindings and clears history; do it outside the callback.

Continue zero-input render blocks until `renderer.drained()`. `busy()==0` alone is insufficient because pending output may remain. For an optional separate shared-XTC matrix, continue its own zeros for at least `xtcTaps-1+128` additional frames after the object stage drains, rounding to full blocks. Alternatively precombine HRIR and XTC FIRs, retain every resulting tap and use the fused bank file; that removes the second 128-frame buffer while increasing per-source work.

All coefficient/storage preparation and file I/O stay outside rendering. Nearest-direction lookup, gain interpolation and source-control conversion can run inside rendering if required by incoming metadata, but their work must be included in the COM-host timing measurement. The scale benchmark measures renderer/lifecycle callbacks and prepared-bank transitions; it does not include the nearest-grid search or COM/engine overhead. No room/reverb processing is added by this kernel.

The 128-frame kernel delay is distinct from the native Source[] callback's submission quantum, wrapper buffering and device latency. A 1,024-frame source callback cannot establish 128-frame end-to-end latency merely because it contains eight internal renderer calls.
