> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# SOFA replacement inside Sonic's native object renderer

The investigation is now restricted to replacing the listener/HRTF processing behind Windows Sonic while retaining its spatial-object input and native source controls. Direct coefficient replacement remains the preferred route; replacing the DSP or complete renderer is the fallback. DTS decoding, channel conversion and crosstalk cancellation are outside this focused work.

## New native proof

An actual full-sphere SOFA now drives the real, pinned HrtfApo engine in an isolated process. This is a stronger result than the earlier diagnostic filter.

- Loaded all 710 positions, both ears and all 4,096 samples per response directly from `compatibility_length_4096.sofa`, SHA-256 `<private-artifact-digest>`.
- Created two sources through Sonic's native engine factory. Set both source positions using the real native control interface, not replacement position fields supplied by the test callback.
- Replaced only the isolated engine instance's DSP processing method. The native engine continued supplying the separate mono buffers, positions, gains and source frame counts.
- Tested every SOFA measurement direction with two simultaneous sources and different impulse times. Native positions selected the expected SOFA responses for both sources in all 710 cases.
- Compared every output sample with an independent summed-convolution reference. Largest absolute difference across the cases: `8.124e-10`.
- Repeated ten cases under WinDbg/CDB using the actual interpreter rather than its child-spawning virtual-environment launcher. A breakpoint hit the real native position setter at HrtfApo RVA `0xe2c0`; the native result file confirms successful output comparisons.
- Restored each instance's original DSP method table before releasing the engine. No system file, endpoint setting, service configuration or default device was changed.

The full result is native_sofa_full_sphere_20261016/result.json. The debugger evidence is the native-control trace and its result. The executable experiment is sonic_sofa_engine_probe.py.

This prototype reads the SOFA responses directly and performs source-specific convolution inside the native engine call. It does not first turn the scene into a fixed channel layout or process Sonic's already-mixed stereo output.

## Interface and coefficient findings

Fresh Ghidra analysis identifies source interface `15b3cd66-e9de-4464-b6e6-2bc3cf63d455`. Its slot 3 is the position setter at HrtfApo RVA `0xe2c0`. The engine's existing GetControlInterfaceAt operation returns this interface for each configured source. The setter's recovered body copies the three position floats, updates source distance and publishes the source parameter buffer.

The source DSP record's first three floats are the supplied position, followed by gain. The next 16 bytes remain semantically unmapped in this work. They must not be assumed to be a second/end position merely because the earlier diagnostic structure gave them those names. The actual SOFA experiment uses the verified first position/gain fields and leaves that block uninterpreted. Native PCM pointer and frame count remain at offsets 104 and 112.

The inspected default CPU implementation still has no exposed variable-length coefficient setter. Its legacy dataset interfaces are ignored at DSP creation, and its filter processing reaches the embedded descriptors. Updating DefaultHrtfs.bin therefore remains an unestablished route for the default CPU implementation. Replacing the DSP now has a working full-SOFA execution proof, rather than only an inferred possibility.

Fresh setter/interface evidence is retained in `2026-10-16_source_position_setter.json`, `2026-10-16_source_interfaces.json` and `2026-10-16_source_constructor.json`. The filter pipeline is retained in `2026-10-16_embedded_filter_pipeline.json`. These identify the inspected binary and Ghidra evidence IDs.

## What still prevents a default Windows replacement

The isolated instance substitution is not Windows installation or system-wide selection. A normal application currently still gets the installed Sonic renderer.

The default path explicitly loads HrtfDspCpu from System32. The alternate COM factory branch exists, but the inspected desktop selection path gates its override lookup behind the unavailable API-set contract described in the earlier report. A selectable replacement cannot be claimed merely from registering an arbitrary new factory or format GUID.

A read-only check of the active audiodg process found protection byte 0, but OpenProcess with query-information/VM-read access failed with Windows error 5. Limited-query access succeeded. The current execution token is not an administrator, and the installed CPU DLL is owned by TrustedInstaller. These observations constrain live inspection and installation from this session; they do not prove that a privileged, endpoint-specific integration is impossible. No security setting, ownership or process protection was changed.

The next integration target is an endpoint-scoped load/activation route that can insert a replacement DSP factory before Sonic engine construction while preserving the native object frontend. A custom APO as a loader is only a candidate: its load order, object-graph compatibility, signing requirements and restoration behavior have not been verified. A conventional post-mix APO by itself would not satisfy this goal.

The prototype currently uses nearest-measurement selection and was validated at all exact SOFA measurement directions. Smooth continuous movement, interpolation, larger source counts and sustained real-time operation remain separate implementation requirements. It accepts the tested 48 kHz, spherical-coordinate, zero-Data.Delay SOFA; general SOFA compatibility has not been claimed.

Native Sonic room preservation is also unverified. The callback reproduces the full supplied SOFA response tail, but it bypasses the native CPU acoustic processing. Retaining Sonic's own room algorithm would require isolating the HRTF-only stage, or a replacement renderer would need an explicit room implementation. The working HRTF test is not evidence that room controls already remain functional.

The next work is therefore to isolate the native HRTF/room boundary and establish a real Windows loading route. The required acceptance result is a normal Windows spatial-object stream, with Sonic selected, producing the chosen SOFA response at multiple arbitrary positions; passing the isolated experiment alone does not meet that result.
