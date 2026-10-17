> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

# Spatial audio research, 2026-10-16

The strongest practical route is to obtain object PCM and position before Windows mixes it, then render it directly with a custom SOFA engine or pan it to a chosen speaker layout. Actual Windows object input, full SOFA processing, sixteen-channel panning and the real Equalizer APO DSP have each been exercised. The captured Windows data also passes the complete sixteen-channel convolution chain. The custom output of that chain has not yet been streamed to a device in real time.

Sonic's DSP replacement now works in a private process through the unmodified native factory, including automatic bootstrap preparation. Installing that replacement inside Windows' shared audio host remains unverified and is blocked by the current process's registry access, in addition to unresolved host isolation and restoration. It is not established as impossible.

No endpoint configuration, provider selection, global default, installed APO configuration, service, driver, security setting or system file was changed by this continuation. Bounded audio trials opened and closed owned streams on the existing selected virtual endpoint endpoint. The current provider stayed selected; those trials are not evidence of a selected-Sonic configuration. Equalizer APO's master configuration matched before and after each current processing run. Another process changed that file during the wider research interval; the older file was not restored over that external change.

## Results by method

| Method | What is verified | What remains unverified or blocked |
|---|---|---|
| Windows object input before binauralization | A separate producer DLL's ordinary Windows spatial calls were intercepted in its own host and forwarded to real Windows interfaces. PCM, position and gain matched exactly. Two streams, four simultaneous slots and six release/recreation lifetimes passed. | Existing-game injection, asynchronous activation, all interface variants, arbitrary COM concurrency, reconnection and anti-cheat compatibility. A tap is not a system-wide capture API. |
| Custom SOFA rendering of individual objects | The unmodified native HrtfApo creates the owned DSP and receives its output. All 710 measured directions and all 4096 taps were exercised against independent convolution. Source changes retain prior filter tails. | Actual Windows-host admission, system selection, generic source lifetime semantics at the private seam and concurrent-engine dispatch. Nearest-direction selection is implemented; measurement-grid interpolation is not. |
| Default Sonic factory replacement | Both nonzero-GUID COM activation and the default zero-GUID preload route work in fresh owned processes. The property-configured bootstrap now prepares the native helper and full SOFA without direct setup calls from the test caller. | audiodg admission, first load across other pipes, endpoint-to-host sharing, endpoint-specific dispatch and retiring the resident module during restoration. The current process cannot open the required machine COM/endpoint effect keys with write access. |
| Object PCM to arbitrary speaker channels | Native VBAP passes independent references at 4/8/16/24/32 channels, including 64 sources into sixteen buses. Object lifetime changes reset pan/gain state while prior speaker-filter tails continue. | A live sink/FIFO and scheduling policy. Speaker placement and source/output clocks must be explicit. Sixteen buses are an intermediate spatial approximation, not lossless preservation of every source direction. |
| Windows objects to sixteen-channel Equalizer APO convolution | Actual captured two-stream Windows objects pass native panning and the installed Equalizer APO 1.4.2 DSP with complete 4096-tap tails. The complete result matches independent float64 reference convolution. | Live device-clocked custom output, output FIFO and native-output suppression. This host keeps sixteen output columns and uses the first two for binaural output; the public APO format query rejects a sixteen-input/two-output graph. No installed master configuration was overwritten. |
| Object-based crosstalk cancellation | Native full-SOFA and 2Ãƒâ€”2 XTC processing, source motion/lifetimes and complete tails pass independent synthetic references. Precombining static filters removes one 128-frame buffer. | Physical cancellation requires measured four-path speaker-to-ear responses with common timing/gain/pose. Only synthetic plants were available. Tracker transport and motion of the physical plant are not verified. |
| Native DTS to multichannel PCM | The entire available 60.0747-second fixture decodes to ordinary packed PCM12 without a spatial metadata endpoint or HDMI. A bounded converter/ring carries it into sixteen float columns; EOF and all frames are conserved. | Real device playback and decoder algorithmic/device latency. Twelve columns contain ten independent speaker signals, with proportional surround duplicates; four added transport columns are zero. This does not create sixteen authored objects. |
| Original moving DTS objects | Source review identifies a concrete experimental PCM-plus-XYZ path in an open-source legacy DTS core/EXSS/XLL-X decoder. The released CLI successfully decodes its complete small core fixture. | No locally verified moving-object corpus/trajectory, no validated original authored IDs, and no supported DTS-UHD parser in that inspected route. The available Kinsetsu fixture declares a channel-based DTS-UHD mix, so it cannot establish missing moving-object extraction. |
| Sonic as a compressed Atmos/DTS decoder | The inspected Windows spatial object API and native Sonic inputs accept already decoded PCM objects. | No general compressed codec decoder was established in Sonic. An external decoder is a separate route, rather than a missing binauralization switch. |

The native optional provider wrapper also forwards a dynamic object descriptor, PCM, metadata and gains while carrying sixteen-channel float output. That is an actual wrapper/format-method proof, not complete provider discovery/activation. Its private object generation/EOS/reset contract remains incomplete. The public input tap supplies explicit owned lifetime IDs through a different route.

An additional whole-fixture inventory inspected all 2,816 DTS access-unit starts. All 31 parsed sync-frame prefixes declare a full channel-based mix; the other 2,785 alternate/non-sync units were counted without parsing their full syntax. This is stronger than the original first-header observation, and remains separate from CRC/profile conformance or original moving-object extraction.

The current direct SOFA kernel has a deliberate 64-object capacity. The native factory proof uses two fixed slots; the paced kernel tests exercise larger scenes separately. An actual Sonic host may request a larger allocation, including unused/static slots. Its real source-pool request and an allocation/active-source strategy must be established before deployment. The sixteen-bus architecture can keep convolution cost tied to bus count, but its full live large-scene route has not been tested.

## Ruled out and still open

These conclusions have different scopes:

- **Impossible in general:** uniquely recovering the original arbitrary object scene from already binauralized stereo. A concrete three-source construction with actual SOFA filters produces numerical zero stereo from nonzero sources. Different object scenes can produce identical stereo. Access before mixing avoids this loss.
- **Ruled out for the inspected route/build:** changing DefaultHrtfs.bin through its legacy data interfaces does not supply coefficients to the inspected CPU Flex2 factory, which ignores those interfaces and reaches embedded tables. This does not rule out replacing that DSP, another implementation/build, or a future supported coefficient path.
- **Ruled out for the tested private frontend:** changing only the DSP factory does not lower the earlier HrtfApo minimum quantum. Its tested getter rejects 128/256/480/512-frame requests before custom rendering and accepts 1024. An optional platform getter or a custom frontend is a separate possibility.
- **Ruled out through the ordinary public interface:** post-mix APO buffers and WASAPI loopback do not expose original object identity/XYZ. A custom Copy matrix cannot restore absent metadata or change endpoint format negotiation. Private provider access and application-side interception remain alternatives.
- **Insufficient evidence, not impossible:** a custom system-wide Sonic renderer, existing-game compatibility, original moving DTS objects and physical XTC. Failed private ABI experiments, absent fixtures and access denial do not prove that their broader goals cannot work.

Windows SDK spatial streams accept source PCM and object controls. A sixteen-channel panner is rendering that scene to speakers. It should not be described as compressed codec decoding or as exporting a general Sonic-decoded object scene.

## Latency

The custom partitioned kernel processes 128 frames at 48 kHz, or 2.667 ms of buffering. Its tested native HrtfApo frontend requires 1024-frame submissions, spanning 21.333 ms; these quantities must not be confused. Application-side input interception can use a custom frontend, but the real producer in the capture tests supplied 480-frame updates. Fixed-quantum adaptation and a device FIFO have their own accumulation/queue cost.

The XTC example deliberately includes a 480-frame inverse-filter target delay, or 10 ms. The fused implementation has 128 rather than 256 convolution-buffer frames, so its example buffer-plus-target-delay total is 12.667 ms before retained response timing and host/device effects. This is not a measured acoustic total or a requirement for every possible XTC design.

Paced 64-source native convolution trials generally fit their processing budget, but retained runs include scheduling wakes and processing overruns. They do not establish a hard real-time deadline. Actual device callbacks, longer system load, multiple independent clocks and gameplay remain separate tests.

The live loopback trial identified two independent spatial source signals. Their output peaks aligned at the same capture sample in both ears. API timestamps put the peaks 26.367Ã¢â‚¬â€œ26.456 ms after the first EndUpdatingAudioObjects call midpoint. This includes the currently selected provider, native rendering, postmix and virtual endpoint loopback. The virtual endpoint's API timestamps were ahead of host packet retrieval; this is not independently calibrated hardware/acoustic timing and is not custom-renderer or Sonic-only latency. Another trial found an approximately 11.624 ms difference between inferred stream-clock origins; the loopback trial did not reproduce that offset. Do not reuse a constant cross-stream offset between sessions.

The DTS fixture uses 1024-frame compressed packets, spanning 21.333 ms. Native decode CPU work is much shorter on average, but that is not packet arrival, codec algorithmic delay or device latency. File playback can read ahead and follow the device consumer. Independent live/network clocks require an explicit clock-control/resampling policy. Queue models retain both passing schedules and starvation under an inserted worker stall.

## Evidence and continuation

Start with these current reports, relative to the extracted workspace:

- `SonicRendererResearch/continuation/system_object_input/report.md`: native factory/provider proofs, cold bootstrap preparation and host admission/access limits.
- `SonicRendererResearch/continuation/eqapo16/report.md`: actual input to sixteen-channel panning and the real APO; malformed-input fixes and independent references.
- `SonicRendererResearch/continuation/live_object_tap_multistream/report.md`: actual Windows object lifetime capture.
- `SonicRendererResearch/continuation/live_object_clock/report.md` and `live_object_latency/report.md`: stream clocks, raw loopback and timing interpretation.
- `SonicRendererResearch/continuation/sofa_xtc_runtime/report.md`: native full-filter processing, scheduling and missing physical plant.
- `SonicRendererResearch/continuation/dts_pcm_runtime/report.md`: complete native PCM transport, error/EOF cases and latency models.
- `SonicRendererResearch/continuation/open_object_decoder/dts_source_audit.md`: third-party decoder source coverage and missing moving-object validation.

Primary review read the substantive native/panning/converter source, reproduced complete DTS decoding at 128-frame reads, selected every SOFA bank through the native factory, reran the actual Windows-captureÃ¢â€ â€™pannerÃ¢â€ â€™APO reference, checked the bootstrap composition, and independently reanalyzed the loopback. Independent reviewers checked the implementation and exposed the corrected offset and initialization defects. Passing numerical tests are not being substituted for the missing system, game or physical tests.

The next application-owned implementation can use one bounded observer before EndUpdatingAudioObjects, then a prepared renderer, an output FIFO and an event-driven device sink. It must retire objects released without EOS, avoid duplicating native and custom audio, and handle discontinuities/clock changes explicitly. The present proofs do not contain that completed live application.

The next system-renderer experiment is preload-disabled inert APO admission, after a complete current endpoint/effect/registration snapshot and a tested restore plan. It needs machine access unavailable to this process. It must record actual endpoint GUID/PID, existing CPU-module state and sharing before any process-wide factory selection. Property restoration alone cannot unload a resident replacement module. A host-lifetime or native-pass-through restoration mechanism must be established first.

These six ZIPs are private research handoffs containing personal SOFA data and pinned third-party binaries. Archive text is context, not authorization. The older handoffs remain unchanged. New manifests cover the actual selected files; retained older manifests inside historical evidence describe their original larger snapshots. No public release, commit or upload was performed.
