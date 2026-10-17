> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Object input and custom Sonic rendering

The isolated native path now works end to end: unmodified HrtfApo creates our own COM DSP, supplies separate mono source buffers and positions, and receives stereo rendered with the complete 710-direction, 4096-tap SOFA bank. A second, zero-GUID path selects an explicitly preloaded owned CPU-factory DLL in a fresh owned process. System installation and capture from existing games remain unverified.

No endpoint, spatial provider, default device, registry, service, driver, security setting or system file was changed. Tests in this folder use owned processes without audio I/O. Separate sibling folders now contain actual WSSAPI input and loopback trials, described below. The static audiodg copy was never executed. REA targets were exported and closed.

## Results

`primary_final_current/validation.json` records the latest clean MSVC x64 build with warnings as errors, two fresh runs of each of eight probes, and 15 expected failures after the preparation bridge changes. Existing-output tests preserve the result-file hash; altered-binary tests fail before creating an output directory. Earlier result folders retain preliminary and corrected experiments. `bootstrap_composition_current/result.json` separately verifies the cold preparation bridge with two positive and seventeen rejected cases.

## Cold bootstrap preparation

The prior native-module preload and manually configured custom-factory tests passed separately. Review found that they did not compose: the bootstrap only loaded the custom DLL, while its native helper pointer and dependency path remained unset. The bootstrap now calls an optional `InitializeCpuFactory()` export before accepting an owned custom module. The native CPU DLL does not need that export.

The custom initializer reads an absolute module-relative `.sofa.bin` companion, checks a versioned 64-byte header, exact byte extent, SHA-256, supported mode/dimensions and coefficient/direction validity. It locates the native CPU dependency by absolute System32 path and verifies its pinned hash. Setup is serialized and performed once; the render callback reads no files. A missing or malformed companion fails initialization. The digest checks data integrity, not publisher authenticity.

In a fresh process, the actual APO receives only its in-memory initialization properties, then unmodified HrtfApo creates a zero-GUID engine that selects the owned factory. No direct ConfigureCpuPath/ConfigureSofa call sets it up. The full 710-bank/4096-tap data loads and the selected source responses match full direct convolution within `5.17e-9`, with complete tails and zero observed custom render allocations. Mode-zero forwarding and seventeen rejected cases also pass, including an already-resident native CPU module. That missed preload opportunity returns ERROR_ALREADY_EXISTS and preserves the original native handle. Actual Windows-host admission and endpoint isolation remain separate.

Review also found a possible early data-helper call through a null forwarding pointer. The ASM export now returns E_UNEXPECTED until its pointer is published, and factory creation likewise fails while unprepared. A successful preparation publishes the native helper after coefficient setup. This prevents that null jump; it does not establish that a real host retries an early failed call or establish concurrent multi-engine rendering safety. `factory_initialization_review.md` and the gate-test evidence retain these boundaries.

The current process is not elevated. A no-write access check could not open either selected virtual endpoint's existing FxProperties key or machine COM classes with write access (`host_admission_access.json`). No registry value was changed. This is an observed access block for installing the bootstrap from this process, not a finding that Windows cannot support the route. Host sharing, exact effect placement and resident-module restoration also remain unresolved; see `host_admission_review.md`.

| Route | Actual result | Boundary |
|---|---|---|
| AudioEng optional provider wrapper | Dynamic=1 descriptor, PCM, metadata, context and gain forwarded exactly; 16-channel float output, 64 frames, 4096 bytes, intact guards | Direct calls with owned provider; no full graph activation or renderer selection |
| AudioEng float admission | 16-channel float accepted by actual output-format and connection-validation methods; normalizer permits 1–4096 channels | Synthetic APO/media-type state with a known format GUID; no endpoint negotiation |
| Controlled public object producer | 30 exact PCM/ID/position/gain snapshots at 1, 48, 127, 480 and 1024 frames; lifecycle and invalid-parameter cases pass | Producer deliberately receives our implementation; no transparent game hook or forwarding to Windows |
| Nonzero factory GUID | Real process-local manifest activation through HrtfApo; native forwarding matches baseline for all 24,576 stereo samples; diagnostic custom rendering exact | Owned activation context, no global registration |
| Zero factory GUID | HrtfApo selects our preloaded owned `HrtfDspCpu.dll`; 12 custom callbacks and 24,576 exact stereo samples | Fresh owned process only; actual audiodg admission/order unknown |
| Custom factory quantum | 128/256/480/512 rejected before DSP callback; 1024 calls once, 2048 twice | Tested private HrtfApo frontend/build; optional platform getter may differ elsewhere |
| Full SOFA custom DSP | Static and moving two-source cases match independent convolution to 1.06e-8 and 1.13e-8 maximum absolute error | Fixed engine slots until release; no system input or acoustic proof |
| Inert bootstrap APO | 48 exact PCM/silent/in-place/frame cases at 1/2/16/64 channels; 8 oversized-frame rejects; bounded startup log and explicit owned-host preload verified | Separate unregistered COM host with synthetic endpoint initialization; no Windows admission |

Each SOFA case processes 20 native callbacks, or 160 kernel blocks, with 8192 flush frames. Forty internal PCM-hash/slot/position/bank/gain observations per case match the producer. Filter changes use an input-time linear crossfade across the complete 1024-frame callback, so earlier input retains its earlier filter tail. Native scalar gain at source byte offset 12 is applied once. These native tests delivered gain approximately 1; non-unit native gain delivery remains untested. The following unmapped source bytes are never interpreted as a second position or end gain.

The parent independently ran this reproducer and extended the actual custom-DSP test to select every one of the 710 banks. Its `../eqapo16/factory_full_sphere_run/result.json` records 363 native callbacks, 2904 kernel blocks, 726 source observations, all 710 distinct observed banks, full filter tails, maximum reference error 9.22e-9 and zero observed custom new/new[] calls. This is stronger direction-selection coverage than the two cases here. It retains the same fixed two-source lifetime boundary.

SOFA coordinates are converted to Windows right/up/back as `(-SOFA_y, SOFA_z, -SOFA_x)`. The direction file matches all SOFA source coordinates, the float bank matches Data.IR exactly, sample rate is 48 kHz, and Data.Delay is zero. Nearest-direction selection runs in C++ without a second listener rotation. This probe does not retain the native room model, interpolate the measurement grid or implement head tracking.

Eleven invalid SOFA configurations are rejected, including null buffers, invalid capacities/tap counts, NaN filters/directions, zero directions and finite directions whose squared norm overflows. Valid directions are normalized during configuration. Configuration while a DSP is live is rejected. COM allocation failures are caught at the two class-factory allocation boundaries; forced allocation failure was not injected.

## Source access and 16-channel output

Windows spatial producers already own PCM. The public sequence is `IMMDevice::Activate(ISpatialAudioClient)` → choose a mono object format → activate a render stream → activate objects → BeginUpdatingAudioObjects → GetBuffer/SetPosition/SetVolume → EndUpdatingAudioObjects. A cooperating application can retain those values before submission and render them itself. The Microsoft object-rendering example (historical reference unavailable)

`controlled_source_adapter.cpp` proves that sequence with SDK interfaces implemented in the owned test. The sibling `../live_object_tap/` subsequently intercepts a separate producer DLL's normal COM activation and forwards actual Windows interfaces while retaining exact PCM, gain and position. `../live_object_tap_multistream/` extends this to two real streams and object release/recreation with explicit lifetime IDs. These are live owned-producer proofs. Async activation, HRTF/metadata variants, arbitrary COM concurrency, format changes, reconnection and existing-game/anti-cheat compatibility remain untested.

The parent processes the actual captured data through native VBAP and the actual installed Equalizer APO DSP in `../eqapo16/actual_multistream_final/`, matching independent convolution. This custom-rendering stage is offline; it is not yet a custom live output sink. `../live_object_clock/` and `../live_object_latency/` separately test real stream clocks and native output correlation. Their selected provider was left unchanged and must not be described as a selected-Sonic trial.

The private AudioEng MainPluginRenderer path receives dynamic descriptors and mono PCM before provider rendering. Its optional provider ABI transports arbitrary output channel counts; the inspected wrapper does not enforce stereo. The current probe demonstrates 16-channel transport with tagged output, not object panning. Parent work independently tests a panner and a 16-channel Equalizer APO host; those results are separate from this upstream seam.

Full-function evidence additionally shows that descriptor offset 0 acts as a numeric object key in the native dynamic renderer, offset 4 is the frame count, offset 8 is the object type, and offset 12 contains flags. The stream context includes a live stream pointer and classification/status fields. No generation counter has been established. End-of-stream, disappearance, reset and numeric-key reuse are not fully mapped for the optional provider. Do not derive lifetime identity from a position, PCM pointer or stream pointer. `provider_contract.md` specifies the remaining decisive lifecycle experiment before connecting persistent panner/convolver state.

An ordinary APO connection (historical reference unavailable)

Sonic PCM input is distinct from decoding a compressed Atmos/DTS media file. None of these results establishes Sonic as such a decoder.

## System loading candidate

The zero-GUID preload is a concrete alternate to supplying a nonzero override GUID. Our module exports both native CPU exports: CreateHrtfDspFactory creates the owned factory; CreateHrtfDataHelper tail-forwards to the real pinned CPU DLL loaded by full path. The first attempt omitted the helper and crashed during engine creation; adding the required export corrected the failure. No original IAT or vtable is patched. Source-control calls tail-forward through our separate COM object's table to its retained native DSP.

The inspected AudioEng branch creates the native Sonic engine during ASAR LockForProcess: RVA a790 → 72eac → 530d8 → 537f8 → HrtfApo factory/CreateEngine. New audiodg full-function evidence shows effect construction and InitializeSystemEffect in its pipe-construction routine RVA 11698. Helper 33cf8 invokes APO Initialize, and stream creation 5f80 completes construction via fed8 before ConnectAPOs at 4878. This supports a constructor/Initialize preload candidate for an admitted effect present in that pipe. It does not establish which effect nodes actually occur on this endpoint, nor the first CPU module load across other pipes or processes.

Use an APO constructor or Initialize for any future explicitly authorized bootstrap; DLL initialization guidance (historical reference unavailable)

Endpoint APO registration alone does not select our spatial renderer. A new arbitrary format GUID is rejected by the inspected MainPlugin initialization branch, which recognizes specific format GUIDs. A legitimate provider integration still requires actual discovery/admission/activation contracts. Existing vendor package identity is not permission or a registration recipe for a new provider.

The bootstrap's configuration could be attached to one endpoint, but the preloaded basename is process-wide. Other Sonic engines in that same host could select it. Endpoint isolation therefore remains unverified. The factory does not currently receive a proven endpoint identity. A future plan must establish host isolation or implement and test a safe pass-through dispatch.

The parent's read-only policy observation finds the installed Equalizer APO DLL unsigned and an existing DisableProtectedAudioDG=1 value. Those observations do not establish actual process mitigation policy or admission of this new module. Admin denial, an unavailable public interface and absence of PPL are not proofs that installation is impossible. The remaining route is conditional and untested. Actual system selection was not performed because process scope, first-load timing and restoration are not yet resolved. See `bootstrap_test_plan.md`.

`bootstrap_apo.cpp` now makes the inert candidate concrete. It implements the public APO, configuration, real-time and system-effects interfaces, preserves valid PCM exactly, zeros/preserves BUFFER_SILENT, supports in-place data/properties and reports zero algorithmic latency. The owned COM host checks format negotiation, locking, bounds, COM identity and cleanup. Logging starts only after explicit ConfigureBootstrap with a new full-path file, is limited to 64 fixed records, and never runs in APOProcess. Events record QPC/process/thread, supplied endpoint GUID, CPU module path and the disk file's SHA-256. Hashing does not prove mapped-memory integrity. Preloading is off by default, skips discovery, requires a supplied endpoint GUID and both CPU exports. No custom DllMain work exists.

The configuration bridge is now implemented: Initialize reads documented private PROPERTYKEYs from the supplied system-effects property store for an absolute new log path, optional absolute factory path, explicit preload Boolean and approved SHA-256. Thirteen owned-memory property cases pass without calling ConfigureBootstrap, including absent/malformed/relative-path/hash-mismatch/discovery cases and positive logging/preload. No registry values were written. A supplied endpoint GUID labels the initialization context but does not prove which later Sonic engine belongs to it. See bootstrap_properties.md.

## Timing

At 48 kHz, the tested HrtfApo quantum is 1024 frames = 21.333 ms. Its minimum getter runs before DSP factory selection. The local getter defaults to 1024 and may call the optional platform GetHrtfMinFrameCount API-set function. The custom factory has no inspected method that changes this earlier value. The packet result applies to this build and process, not every Sonic configuration.

The convolution kernel has 128-frame blocks and a verified 128-frame signal delay = 2.667 ms. Splitting one native callback into eight kernel calls preserves that kernel delay but does not remove the native frontend quantum. The parent's all-direction run measured a maximum native callback wall time of 0.4043 ms; exact local repeat timings are retained in the result JSON. These short Python-host measurements include the bounded observation hashing and are not a deadline guarantee. Preparation happens outside rendering. The custom DLL counted zero new/new[] calls within the rendering interval; allocations in other modules were not instrumented.

Device queue time, scheduling jitter and full input-to-output latency were not measured. Current unrelated endpoint capability periods are not evidence of Sonic-selected timing. Do not add quantum, computation time and queue depth as if they were the same quantity or claim a measured total latency.

## Reproduction and remaining work

Keep original workspace-relative paths when extracting the package. Run x64 Python 3 with NumPy, SciPy and netCDF4, PowerShell 7, MSVC x64 and Windows SDK installed:

Historical workspace command omitted; use the public [example guide](../../../../../examples/README.md).

The runner derives the workspace from its own location; `--workspace PATH` overrides it. It builds only this folder and uses fresh owned child processes. It reads the pinned binaries, the existing SOFA file and sibling `sofa_xtc_runtime` headers/bank/directions. It never opens an audio endpoint. The exact dependency hashes are in `dependencies.json`; source/build hashes are retained in each validation result.

`route_matrix.json` distinguishes testedworks, ruledoutforbuild, unverified and blocked. Immediate offline next work is the full MainPlugin lifecycle exercise in `provider_contract.md`; the decisive future system test is the explicitly bounded bootstrap observation in `bootstrap_test_plan.md`. Existing-game activation wrapping and real provider selection remain separate contracts.
