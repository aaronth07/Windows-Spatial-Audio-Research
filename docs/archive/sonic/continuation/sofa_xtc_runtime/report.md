> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Moving SOFA rendering and crosstalk cancellation

The isolated native renderer now supports input-time filter transitions, head-orientation coordinate conversion, stable object bindings, creation/removal/reuse with retained tails, and 8/16/32/64-source processing. Four moving-filter cases agree with independent float64 direct convolution. A complete 710-direction SOFA filter pool was prepared and exercised. No live game/system object adapter, audio device, Equalizer APO endpoint or physical loudspeaker cancellation was used.

Current evidence is `results_integration/result.json`, `paced_64_separate_integration.json`, `run_integration.log` and `build.log`. Earlier folders preserve the scalar baseline, optimization passes and intermediate diagnostic runs; they are historical evidence, not the current interface. Final source and input hashes are in `manifest.json`.

## Implemented behavior

`runtime.h` implements a 128-frame, 256-point partitioned convolution renderer. `adapter.h` adds the tested no-allocation nearest-direction lookup, source gain ramps and partial-source-block zero fill. `integration_api.md` documents the exact C++ interface, prepared-bank schema, native coordinate mapping and full-callback transition contract for the separate COM-factory worker. `probe.cpp` is an isolated file-based host and benchmark. `run.py` checks SOFA metadata, prepares coefficients and poses, creates reference input, and compares native output with direct convolution. The copied `dsp.h` supplies the existing FFT and optional shared four-route XTC convolver. The copied `xtc_dsp.py` supplies the unchanged regularized XTC solver. Both copies match their original source hashes.

The time-varying model is defined at input time:

`y_e[n] = sum_objects sum_t x_object[t] * ((1-alpha[t])*h_A[t,e][n-t] + alpha[t]*h_B[t,e][n-t])`.

Prepared filter IDs A/B and the input samples split by the sample-varying weights are retained in the input history. Previous input keeps its original filter and complete tail after a position change. This differs from replacing the filter applied to all old history at output time. Weights are linear within each 128-frame block; the test uses a continuous 256-sample transition across two blocks. No per-source, per-ear or per-filter normalization is introduced.

Each external object ID receives a slot/generation binding. Mix-list order does not determine source identity. Duplicate active IDs, stale bindings, mismatched object IDs, invalid filters, nonfinite audio, duplicate slot submissions and invalid fade parameters are rejected. Removed objects stop accepting input but retain their FIR history. Their IDs can restart in a different free slot while the previous tail drains. Generation values also remain distinct across renderer reconfiguration. Full capacity, including retiring tails, returns an explicit rejected binding rather than stealing a source slot.

`busy()` counts active/retiring object slots. `drained()` also checks the pending output and overlap buffers. A last-sample/last-tap impulse test verifies that a reusable slot can coexist with a pending output tail, and that the final tail is emitted before the pipeline reports drained. For the separate shared-XTC path, continue zero input through the shared matrix for its remaining `xtc_taps - 1 + 128` frames after the object renderer drains, rounding up to a complete block. The reference host flushes both full FIR tails and both buffers and checks all recorded samples.

Filter spectra, history, coefficient transforms, file I/O and storage allocation occur before processing. Callback code uses fixed arrays and bounded loops. Global C++ allocation/deallocation counters observe zero `new`/`delete` operations during all tested render/lifecycle callbacks, including source creation/removal and transitions. Code review found no direct allocation, file I/O, locks or coefficient preparation in that path. These are callbacks in the isolated test host, not Windows spatial-audio callbacks.

The scalar baseline exceeded the processing budget at 64 sources. The correction uses SSE2 complex multiply-accumulate and stores only the 129 independent bins of each real-valued 256-point spectrum, reconstructing conjugate bins before inverse FFT. SSE2 is part of the Windows x64 baseline; AVX is not required. The independent direct reference still passes after this correction.

## Results

| Case | Maximum absolute sample error |
|---|---:|
| Moving HRIR filters | 7.26e-8 |
| Same moving input with complete 710-direction prepared pool | 7.26e-8 |
| Moving HRIR plus shared 2×2 XTC | 7.55e-9 |
| Moving precombined HRIR/XTC filters | 4.56e-9 |
| Precombined moving path versus the original full-chain direct reference | 4.58e-9 |

The reference uses `numpy.convolve` with float64 accumulation and the authoritative float32 samples, coefficients and input-time weights. It includes initial partition latency, source removal, ID reuse, all tails and additional zero flush. All four moving cases finish with zero busy slots and the object renderer fully drained. Host-negative cases reject truncated/trailing input, invalid object IDs and existing output files without overwriting results.

A controlled DC/gain fixture switches between `[0.2, 0.8]` and `[0.8, 0.2]` ear gains over 256 input samples. Its maximum adjacent-sample change is 0.00235325 versus the analytical `0.6/255 = 0.00235294`, within the stated FFT tolerance; an abrupt replacement changes by 0.6. This verifies the transition law and bounded coefficient-change artifact for that fixture. It does not establish perceptual quality, and the adapter does not invent start/stop fades for source PCM.

Head coordinates use +X front, +Y left, +Z up and a Hamilton xyzw quaternion rotating head coordinates into world coordinates. The control-side transform applies the inverse rotation to `source_position - head_position`; it normalizes valid quaternions and rejects invalid/coincident poses. Tests cover identity, +90° left yaw, quaternion sign equivalence, listener translation and pose-to-prepared-filter selection. A world-front source becomes local right under a +90° left head turn, selecting the right-side SOFA bank. This verifies coordinate handling and filter selection offline. A live tracking transport and continuous pose-to-filter update queue remain to be connected.

The SOFA file is `<local-capture-file>`, SHA-256 `<private-artifact-digest>`. It is a renderer-derived 48 kHz, 710×2×4,096 SimpleFreeFieldHRIR/FIR dataset with zero separate Data.Delay. Ear order, coordinate units, listener axes and 1 m source sphere are checked. All HRIR timing and relative levels are retained. Lookup uses nearest sampled direction; transitions mix prepared directions rather than claiming spherical interpolation. Listener translation is a coordinate transform only; near-field distance responses and propagation/level extrapolation are not supplied by this dataset.

## Scale, timing and memory

Every unpaced case processes 768 windows. Sources use independent prepared-bank indices, with adjacent banks active simultaneously in every block and changing bank pairs every four blocks. The 710-pool case uses the full bank count rather than restricting processing to four directions. Each processing window has 2,666.667 µs available at 48 kHz / 128 frames.

| Mode | Sources | Mean µs | p99 µs | Max µs | Budget exceedances |
|---|---:|---:|---:|---:|---:|
| hrir | 8 | 86.0 | 143.0 | 306.6 | 0 |
| hrir | 16 | 178.8 | 332.5 | 351.9 | 0 |
| hrir | 32 | 390.5 | 635.6 | 751.0 | 0 |
| hrir | 64 | 723.8 | 1130.2 | 1224.2 | 0 |
| separate_xtc | 8 | 153.7 | 215.5 | 271.7 | 0 |
| separate_xtc | 16 | 244.0 | 369.3 | 382.4 | 0 |
| separate_xtc | 32 | 460.2 | 681.4 | 3095.8 | 2 |
| separate_xtc | 64 | 735.0 | 1163.4 | 1237.0 | 0 |
| fused_xtc | 8 | 136.4 | 212.5 | 257.7 | 0 |
| fused_xtc | 16 | 267.4 | 452.7 | 490.0 | 0 |
| fused_xtc | 32 | 603.2 | 957.0 | 1133.5 | 0 |
| fused_xtc | 64 | 1068.0 | 1709.3 | 1926.9 | 0 |
| Full 710-bank HRIR pool | 64 | 842.6 | 1255.4 | 1508.2 | 0 |

| Paced trial | Windows | Mean µs | p99 µs | Max µs | Processing exceedances | Wake lateness > quantum |
|---|---:|---:|---:|---:|---:|---:|
| 16 sources, shared XTC | 750 | 322.2 | 675.8 | 3578.7 | 2 | 0 |
| 64 sources, fused XTC | 1500 | 1244.8 | 1865.5 | 2439.2 | 0 | 1 |
| 64 sources, shared XTC | 1500 | 801.1 | 1256.2 | 1437.2 | 0 | 2 |

The final paced runs use ordinary Windows scheduling and `sleep_until`, without audio devices, priority changes or MMCSS. Wake lateness counts starts more than one entire quantum after their intended start. Earlier optimized runs retained isolated wall-time budget exceedances, including six 64-source fused overruns in one paced trial and one 64-source separate overrun in another. The final run does not erase those observations. No scheduler trace or per-thread CPU-time attribution was collected, so their cause is unassigned. These trials establish processing feasibility on this machine, not a hard real-time or device-underrun guarantee.

At 64 slots, HRIR input history and metadata occupy 4,259,840 bytes; fused 8,191-tap history uses 8,519,680 bytes. Four prepared HRIR banks use 264,192 bytes of spectra; four fused banks use 528,384 bytes. The shared XTC matrix adds 393,216 bytes of spectra/history. The complete 710-direction HRIR pool uses 46,894,080 bytes of prepared spectra; its 64-source test has the same 4,259,840-byte input history. These counts exclude source-file storage, ordinary object/stack buffers and total process overhead. Capacity is fixed at configuration; filter-pool storage is checked against a 512 MiB limit before allocation.

The separate mode uses 4,096-tap HRIRs followed by one shared 4,096-tap 2×2 XTC stage. The fused mode convolves each HRIR with the same XTC matrix in advance and retains all 8,191 taps. It needs more per-source work/history but removes one 128-frame buffer. Both modes apply the same single common output gain, and their moving-source output agrees after correcting the 128-frame shift.

| Latency component | HRIR only | Shared XTC | Fused XTC |
|---|---:|---:|---:|
| Object convolution buffer | 128 frames | 128 frames | 128 frames |
| Additional XTC buffer | 0 | 128 frames | 0 |
| XTC inverse target delay | 0 | 480 frames | 480 frames |
| Buffers plus inverse target | 2.6667 ms | 15.3333 ms | 12.6667 ms |

HRIR response timing remains in the filters. The inverse delay is a transfer-target phase delay, not the exact first-nonzero output sample. Device, driver, transport, host input queues and sensor latency were not measured. In a real host, those components must be added separately.

## Method status and integration boundary

| Method | Status and remaining work |
|---|---|
| Custom SOFA object rendering | Implemented and independently verified in the isolated native host, including the full prepared 710-direction pool |
| Moving source/filter updates | Implemented as input-time crossfades between prepared banks; live metadata transport remains an adapter task |
| Head orientation | Coordinate transform and prepared-filter selection verified; live sensor transport and acoustic plant updates remain open |
| Stable object lifecycle and tails | Implemented with generation bindings, retirement, explicit pipeline drain and capacity rejection |
| Two-speaker XTC | Native shared and fused paths verified against direct convolution using a synthetic plant |
| Real acoustic XTC | Unverified; no measured four-path plant found in the bounded inventory |
| Windows/game object acquisition | Not implemented by this worker; this DSP result neither proves nor disproves a capture route |
| Arbitrary N-speaker output inverse | Not implemented; current output is two ears or two physical speakers |
| 16-channel Windows/EQ APO transport | Not established by this worker; channel negotiation, format/order and endpoint routing belong to the root investigation |

For a 16-channel linear-PCM adapter, preserve one explicit layout and interleave order, sample rate and frame count. Convert a `frames × 16` float32 block into the renderer's channel-major 128-frame source blocks, retaining one stable binding per channel. Assign source directions from the actual fixed channel layout, including height-channel elevation when applicable. This renderer can then consume those 16 channels as fixed virtual sources and produce two ear targets or two speaker outputs. It does not expose 16 output channels.

Linear PCM alone does not carry independent object IDs, positions, lifetimes or head poses. A fixed 16-channel bus therefore remains a channel render unless metadata is supplied separately. Do not infer moving object capture from a working multichannel PCM connection. For a general 16-channel convolver, use `output[o] = sum_i FIR[o,i] * input[i]`, preserve full tails and apply one common gain after the route sums. Actual Equalizer APO matrix-file/configuration support and Windows 16-channel endpoint negotiation require their own tested adapter; this report provides no untested configuration syntax.

The bounded filename inventory covered the workspace while excluding handoffs, repositories, this continuation folder, build-object and node_modules trees. It found the `XTCStudio/examples/synthetic-48k` plant pair and the compiled profile-test fixture. Both profile metadata records declare `plant_kind: synthetic`; the compiled fixture has no plant WAV pair. No physical cancellation claim is possible from those inputs. This is a bounded search, not proof that no relevant measurement exists elsewhere.

A measured plant must contain the left and right ear responses for each of the two physical speakers: `H[sample, ear, speaker]`. The existing solver accepts a stereo left-speaker IR WAV and a stereo right-speaker IR WAV, both at the same rate, with ear order left/right and a common timing/gain reference. Preserve propagation delay and relative levels across all four paths. Repeat measurements at the intended listener pose, record the usable noise floor and amplifier/headroom limits, and validate the realized speaker output independently at both ears. Physical head movement changes that plant; rotating the object HRIR alone does not establish valid head-tracked cancellation.

## Reproduce and review

Prerequisites are Windows x64, MSVC C++17 plus Windows SDK, and Python with NumPy, SciPy, netCDF4 and soundfile. Keep this folder at its workspace-relative location and run from the workspace root:

Historical workspace command omitted; use the public [example guide](../../../../../examples/README.md).

Choose new result paths; the runner and native host preserve existing outputs. The Python environment used for these runs is not packaged. `--workspace` and `--output` accept explicit paths. `--blocks`, `--paced-blocks` and `--paced64-blocks` set trial lengths; the native host bounds these dimensions.

The second pass reviewed time-varying filter semantics, coefficient/channel order, Hermitian spectrum reconstruction, sample weights, whole-tail flush, generation reuse across resets, source-list ordering, finite/range checks, coefficient-pool bounds, callback allocation behavior, independent directions in the source-count benchmark, common gain placement and latency accounting. A unit-amplitude last-tap impulse exhibits about 3 ppm float-FFT error and passes the same 4e-5 absolute reference tolerance used by the other native tests. All original sources and the earlier handoff remain unchanged. No playback, device/provider/default, registry, service or driver changes were made.
