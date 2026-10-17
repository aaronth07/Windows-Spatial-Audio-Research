> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Independent Equalizer APO and VBAP review

No blocking numerical or 16-to-stereo routing bug remains in the reviewed implementation after the root worker's fixes. This review read the five requested source files, the relevant Equalizer APO source paths and the recorded results. It did not edit the root worker's files or repeat its full suite. `reviewed_sources.json` and `after_fix_sources.json` identify the inspected file versions; source snapshots are retained beside this report.

## Confirmed findings and fixes

| Finding | Final review |
|---|---|
| Positive-case worker failures could pass the Python runner | Fixed. The 12 general positive cases now require both zero worker exit and a passing reference. The original condition only rejected a wrong numerical result after a successful worker exit. |
| FixedQuantum constructor multiplication overflow | Fixed and independently rerun. `extent()` rejects zero dimensions and `count > SIZE_MAX/frames` before vector construction. The native probe accepted `(SIZE_MAX/2+1, 2)` before the fix and rejects it afterward. No unsafe push was attempted. |
| BUFFER_SILENT output was copied as ordinary PCM | Fixed. The host now writes zero samples to its result stream for a silent output flag. The pinned DLL already writes zeros in the tested path, so this corrects host semantics without changing those reference results. Microsoft defines BUFFER_SILENT as data that must be treated as silence. APO_BUFFER_FLAGS; the installed master hash remains unchanged. Six VBAP cases pass, and the real APO full-SOFA matrix chain retains maximum direct-reference error 1.9716549417431173e-9. The root also records its separate negative-input suite. Those are root-produced runs, distinct from the independently rerun small probe above.

## Numerical and lifetime assessment

The VBAP face inverse is correct: its rows are the cross products divided by the scalar triple product, solving the matrix whose columns are the speaker directions. The Python reference independently uses a linear solve. Positive triplet selection, small negative tolerance, zero clipping and L2 gain normalization are consistent with the declared target model. Generated layouts explicitly place the listener strictly inside their convex hull, so the tested spherical layouts cover all directions. Native input validates finite unit speaker vectors, indices and nondegenerate triplets.

The per-object gain transition follows the stated policy: interpolate previous/target panning weights over 128 samples, normalize the interpolated gain vector, and apply the independently interpolated source gain once. Coordinates are listener-relative SOFA coordinates: +X front, +Y left, +Z up. This is not the Windows right/up/back coordinate convention; a native Windows metadata adapter must convert it explicitly.

The direction-reconstruction metric is computed for each block's target weights, before sample-wise transitions. It does not bound intermediate directions or establish a requested angular trajectory throughout the crossfade. A clearer report label would be `maximum_target_direction_reconstruction_error`. The current results correctly show endpoint geometry and unit panning power; they do not establish physical localization or perceptual equality with direct per-object HRTFs.

The 16-to-stereo convolution routing is consistent. IR columns flatten in `(source, ear)` order, matching `LEFT1 RIGHT1 ... LEFT16 RIGHT16`. Copy operations create the 32 temporary routes, their full FIRs are convolved independently, the left/right sums return to physical outputs 1/2, and outputs 3–16 are zeroed. The padded stream contains the entire 4,096-tap tail. Direct reference agreement supports the actual pinned Equalizer APO convolution path, not just a mock convolver.

Normal COM release order is correct: APO/media/configuration/factory objects leave scope before the Imports guard. The HMODULE deliberately remains mapped until worker exit, preventing premature DLL unload. The host does not use a long-lived reusable COM-apartment lifecycle; its process exit is part of its isolation contract.

The read redirects affect the loaded EqualizerAPO image's IAT slots only. They do not patch the on-disk DLL, install an APO, modify the system registry or redirect all dependency/system-module calls. The dummy device and failed endpoint registry reads prevent this test from demonstrating a real endpoint chain. The result is evidence for the real DSP under a research configuration, not an unmodified audiodg/device-load path or a global I/O sandbox.

## Limits that should remain explicit

The silent-tail counterexample is specific to this research host. The source initializes `allowSilentBufferModification=false`; failed device-info lookup leaves that policy unchanged. Equalizer APO also has a true branch that preserves nonzero filter tails and returns BUFFER_VALID. Therefore the result does not prove that every Equalizer APO installation suppresses silent-buffer tails. Sending valid zero samples through the isolated host is the tested conservative remedy.

FixedQuantum coalesces arbitrary producer packets into fixed processing quanta. It does not produce exactly one equal-size output packet for every incoming callback, provide an output FIFO or automatically finalize a partial final quantum. Callers must pad the final remainder and supply the required FIR tail zeros. A live callback adapter also needs an output FIFO and must account for packet accumulation delay. APO-reported latency zero does not remove those transport/device delays.

Changing the convolver frame count is a limitation of this pinned path, not a general inability to adapt variable producer packets. The successful adapted case holds the APO quantum constant while preserving sample order and tails. The source also explains why a smaller first callback is unsuitable here: it can rebuild convolution filters on that first processing call, including allocation and file/FFT preparation.

Zero `operator new`/`operator delete` observations in the separate SOFA renderer are not proof of zero malloc/realloc/free/HeapAlloc activity, allocations in dependent DLLs or all activity in the actual APO chain. The VBAP valid path uses prepared vectors/reserved timing storage by inspection, but this review did not add allocator interception to it. Any allocation claim should identify the instrumented module, functions and tested path; the Equalizer APO chain has no complete heap-allocation measurement in these results.

No further feature changes or full-suite reruns are needed for this bounded review. A live WSS/16-channel endpoint connection, real-time output transport, physical acoustic cancellation and perceptual equivalence remain separate integration/measurement questions.
