> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# CPU factory preparation review

The initialization bridge and startup guards now work in the tested fresh owned processes. Root owns the DLL changes. This review made no DLL edits, rebuilds, endpoint changes or live audio calls.

`bootstrap_composition_current/result.json` records two positive and seventeen negative cases. Property-store Initialize alone prepares the copied factory and creates a zero-GUID HrtfApo engine in forwarding and SOFA modes; no direct ConfigureCpuPath/ConfigureSofa call is required. The SOFA case prepares 710 banks of 4096 taps, preserves the full tail, reports maximum reference error 5.165325560503586e-9 and zero renderer new calls. Native-first startup is rejected with ERROR_ALREADY_EXISTS rather than accepted as custom selection. Missing/malformed sidecars, extent/hash failures and invalid coefficients/directions are rejected.

## Startup entry points

The earlier unconditional null-target helper jump was a concrete source defect. Root replaced it with a single pointer read and E_UNEXPECTED when unprepared. InitializeCpuFactory is serialized and idempotent; an already manually configured factory is rejected before preparation side effects. ConfigureCpuPath prepares its string before loading, catches exceptions and publishes the helper pointer atomically after assigning the native path.

`preparation_gate_test.py` and canonical `preparation_gate_ready/result.json` independently exercise the built module in a cold child:

- The raw helper called only before preparation returns E_UNEXPECTED without a crash. Raw factory mode0 also returns E_UNEXPECTED with null output; null output and mode1 reject safely.
- Each of two MTA workers performs eight pre-start calls and 200 preparation-window calls. All 416 observed calls return E_UNEXPECTED. This run does not observe a successful factory call during preparation.
- After successful initialization signals an event, both workers meet a ready barrier and each creates/releases 32 factories: 64 S_OK results and 64 releases. Repeated initialization returns S_OK; live DSP count stays zero.
- Inspection of the owned thunk's pointer slot identifies the published helper's actual owning module as System32/HrtfDspCpu.dll, with SHA256 4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9. Its address equals that module's CreateHrtfDataHelper export. No successful helper call with unknown ABI was attempted.

The tested custom DLL SHA256 is <private-artifact-digest>. Earlier preparation_gate_current remains historical: its post-ready factory calls were serial on the main thread. The ready result is the canonical two-worker readiness test. These are factory startup tests, not concurrent DSP/engine or actual audio-host retry tests.

## Bounds and ownership

The source review covers factory_initialization.h:13–58, regfree_factory.cpp:137–165, regfree_forwarders.asm and bootstrap_apo.cpp:89. The 64-byte header validates magic/version/mode/reserved fields and exact payload extent. Forwarding requires an empty payload; SOFA requires 1–2048 banks, 4096 taps and capacity1–64. These bounds fit the 128MiB input and ULONG hash limits on x64. SHA256 checks integrity, not publisher authentication. ConfigureSofa validates finite coefficients and finite nonzero direction norms, normalizing directions outside rendering. Exceptions become HRESULTs at exported preparation boundaries.

The sidecar is resolved beside the module identified by its function address, without CWD/environment lookup. The native dependency uses an absolute System32 path and pinned hash; the gate experiment additionally verifies actual loaded helper ownership. Bootstrap holds a module reference during preparation. Native helper state and factory dispatch remain process-wide; production unload/restoration and generic concurrent DSP operation are unproven. This initializer is not a transactional reconfiguration API. Capacity64 still needs comparison with a real host's static/dynamic source allocation.

A second-pass source/result review found no additional bounded startup defect. Safe E_UNEXPECTED failure closes the null-jump issue; actual audiodg handling, retry behavior, first-load competition and endpoint isolation remain unresolved in host_admission_review.md.
