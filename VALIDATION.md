# Public snapshot validation

The public source was checked on Windows x64 on 2026-10-16 with Python 3.12, MSVC C++17 and the Windows SDK. These checks apply to this generic public copy; older live results are explicitly historical in the archived notes.

- Six Python tests passed: profile checksum and isolated room-tap edits, malformed payload rejection, sweep calibration, shared stereo onset trim, fractional-octave smoothing and generated SOFA/native-bank parity.
- Seven media chunk-mapping and nine latency correlation cases passed without opening an audio stream.
- The native SOFA/XTC runtime self-test passed with zero instrumented callback allocations/deallocations.
- Six native VBAP configurations passed against independent reference panning: 4–32 outputs and 1–64 input objects.
- Native source builds passed for the runtime, VBAP, Equalizer APO host, single/multiple-stream interception, stream clocks, latency probe, capture worker, DTS PCM transport and private-interface bootstrap.
- Single-stream interception: 16 offline cases passed; multi-stream and clock variants: 19 each; native latency: five cases passed. These tests opened no Windows audio streams.
- The synthetic 710-direction SOFA also passed a causal null-space construction, with maximum binaural residual below 1.5e-17.
- The generic default-device selection builds successfully. It has not been freshly exercised in a live audio trial in this publication task. Archived endpoint-specific captures do not establish that every default endpoint supports the same interfaces.

Generated fixtures are artificial numerical data. No personal profile, capture or vendor DLL is needed for the default suite. Private-interface and installed-library probes remain version-pinned optional experiments. The current public task made no Windows audio configuration changes.

`python tools/check_publication.py` checks the tracked publication files for private path/identifier patterns, unexpected file types and missing local Markdown links. The release is generated from Git-tracked text sources only. Public remote content is independently retrieved and inspected after publication.
