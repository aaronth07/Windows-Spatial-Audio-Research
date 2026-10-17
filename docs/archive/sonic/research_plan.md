> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Windows spatial renderer research — 2026-10-15

Investigate replacing the Windows Sonic listener filters and complete renderer while preserving Windows spatial-object input. Then assess custom decoding, arbitrary speaker outputs, real-time latency and crosstalk cancellation. User authorizes autonomous investigation and bounded experiments; existing browser/library restrictions remain in effect.

| Question | Required evidence | Initial status |
|---|---|---|
| Does this desktop Sonic DSP use DefaultHrtfs.bin? | Matching binaries, factory/data flow, native observation if needed | Prior static evidence points to embedded Flex2 tables; refresh identities and test isolated factory |
| Can the listener coefficients be replaced? | Complete data setter or native coefficient substitution with changed output | No exposed setter established in prior trace |
| Can the full Sonic renderer be replaced? | Loader/COM interfaces, override gate, registration and actual object-buffer ABI | Trace default factory and alternative provider registration |
| Can an app receive other apps' object streams? | Capture-side interface or verified engine/provider boundary retaining object buffers and positions | Public ISpatialAudioClient is a producer-facing render sink |
| Can Sonic decode encoded Atmos/DTS media? | Codec/transport ownership and actual decoder interfaces | Separate codec decoding from rendering PCM objects |
| Can objects be rendered to arbitrary speakers? | Native N-output boundary or a custom object panner before binaural mixing | Inspect output constraints and metadata transport |
| Can crosstalk cancellation follow object rendering? | Preserved object inputs or stereo binaural output, filter model, latency budget | Inspect existing local XTC work; research regularized speaker-to-ear inversion |
| Do home-theater/OEM paths offer useful alternatives? | Installed provider boundaries, MAT/DTS transport, speaker/virtualizer controls | Offline inventory first |
| What is the practical implementation path? | Tested prototype at the smallest isolated boundary, measured processing cost and explicit integration limits | Pending |

Work in this folder and reuse pinned prior evidence under SpatialRendererProfileImport/windows_sonic. Preserve originals. Prefer isolated worker processes and inert copies to audio-service or system-file changes. Before any endpoint mutation, record the exact change, fresh backup and restoration procedure. No publication or messages to other chats are required.

## Completed research pass

2026-10-16 scope update: focus on full-sphere SOFA replacement behind Sonic's native spatial-object frontend. The new direct-SOFA experiment passed all 710 measurement directions with two simultaneous native sources. See `reports/2026-10-16_sofa_renderer_focus.md`. System loading and native room preservation remain unresolved. Earlier DTS/XTC follow-up branches are not part of the active focus.

The final report is `reports/2026-10-15_research_report.md`. Native evidence is summarized in `reports/2026-10-15_verified_native_results.json`; twenty checks passed in `validation/20261016T035319038Z`. The checked endpoint registry snapshots are exactly equal.

- Default CPU path: embedded-table use supported by static analysis; native front/left/right responses verified. Simple dataset-file substitution remains unproven.
- Private replacement seam: alternate DSP factory accepted and custom diagnostic processing verified through the real HrtfApo engine, including WinDbg repetition. System-wide selection remains unresolved.
- Decoder: native DTS spatial output and 5.1 PCM work. The tested spatial output is a fixed 5.1.4 layout; original moving-object extraction remains unproven.
- Speaker/OEM paths: encoder and offload branches identified; general-purpose geometry/XTC controls not established.
- XTC: existing native convolver matched a direct reference; synthetic latency/cancellation tradeoffs recorded separately. No physical acoustic result is claimed.
- Coefficient findings are in `coefficient_findings.md`, separate from renderer integration research.

All new work stayed in isolated workers and research outputs. No live renderer installation or endpoint configuration change was made. Follow-up experiments are prioritized at the end of the report.
