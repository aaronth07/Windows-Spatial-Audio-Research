> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Sonic mapped-data update audit

No event-driven configuration or CPU transfer-function rebuild was established in the selected desktop Sonic path. `InitOnce` retains the configuration's mapping view and length, while each new engine still creates a CPU DSP and source instances. An in-place mapping update could therefore remain visible through the retained view; absence of a reload callback does not prove that every existing or new stream keeps old coefficients. The remaining DSP data-consumption edge prevents an uptake claim.

This pass used separate private Ghidra project `cache_audit/ghidra_projects/SonicCache`, inert local HrtfApo (`ffc12d088cc02441061cc2bb7f3b367fe8089662e0fd61ab98209ef6d43b6489`) and HrtfDspCpu (`4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9`). No shared REA switch, native DLL/audio call, service restart, registry write or configuration change occurred.

## Publication and configuration

| Method/address | Exact observed behavior |
|---|---|
| SSDM0x18000d4a8 | Constructs `HrtfSharedDataInstance`; creates unnamed event via `CreateEventW(attributes,FALSE,FALSE,NULL)`: auto-reset, initially nonsignaled. Stores handle at instance+0. Complete retained function. |
| SSDM0x18000e068 | Requires environment-vector count to match the existing instance. Calls0x18000e004 for primary and environment mappings, copies anthropometric scalar to instance+8, then signals `SetEvent(instance[0])`. Complete retained function, evidence`ev_f93ccf67f42716d1d2d0bcf7bd34899d53311390204a1ba65aed6f828249bb08`. Mapping-copy helper0x18000e004 was not recovered in this pass. |
| HrtfApo0x18000b9e0 | Uses `InitOnceExecuteOnce(&DAT_180022088,0x18000b170,dataset_type,NULL)`. Callback runs configuration creation only if DAT_180022090==0, sets that flag1 after success. Configuration pointer is DAT_1800220c8. |
| HrtfApo0x18000aad4→0x180009ec4 | Allocates configuration0x78 bytes, binds to `SpatialSoundDataManagerRpc` through0x18000e754→0x18000e874 (NdrClientCall3 procedure0), maps primary and environments read-only. Retains primary handle atconfiguration+0x28, view+0x30, length+0x38; environments at+0x40 with handle/view/length records. Count at+0x70; dataset-type predicate at+0x74. |
| HrtfApo0x18000a3bc | Releases the temporary RPC response: closes first handle, primary handle and environment handles still owned by response, then frees the response. Initialization transfers mapping handles/views but does not transfer the first response handle into the configuration. Its exact pairing with the service event requires the RPC server response-field producer. |
| HrtfApo0x180009e50 | Primary-data getter returns configuration+0x30 view and +0x38 length directly; no RPC, remap, wait, generation check or conversion. Configuration's primary vtable is0x180014e68, getter atslot+0x18. Secondary environment interface atconfiguration+8 uses0x180014e28;0x180009e70 returns its existing environment view/length. |
| HrtfApo0x180012d20 | Releases global configuration during cleanup. No selected event path calls it or clears initialization state. |

Complete selected code is retained in configuration/cleanup, initialization and wait callers, and RPC. The earlier service anthropometric gate still applies: dataset/cache report establishes that rereading changed file bytes alone need not cause conversion/publication.

The direct imported wait consumers do not provide a mapped-data notification callback.0x1800043b4 creates/acquires named `Local\\SM0:...` mutex;0x180005ab4 waits/releases mutex state;0x180004f74 probes and adjusts a semaphore count. CPU0x1800dcf20 waits for a thread then callsGetExitCodeThread/CloseHandle. No RegisterWaitForSingleObject/threadpool-wait registration or SetEvent/ResetEvent consumer was found in these two imported-symbol scans. This is bounded direct-import/caller evidence, not a guarantee against all indirect or dynamically resolved synchronization.

## Existing versus new engines

Every successful HrtfApo0x18000b9e0 call proceeds past InitOnce to CreateHrtfDspFactory (zero override), creates DSP through factory slot+0x18, then allocates/initializes source instances via0x18000d724. New engines therefore are not blocked from construction by the configuration singleton. They receive the same cached configuration interfaces; this does not demonstrate acquisition of a new mapping handle or changed length.

CPU's actual `HrtfDspCpuFlex2Factory` vtable is0x180141248, slot+0x18→0x180004c30. The RuntimeClass abstract-base vtable0x1801411b8 has a pure-virtual/abort thunk atthat slot and must not be used as the real factory.0x180004c30→0x180005034→0x180005458→0x180003d54 constructs a fresh `HrtfDspEngine`, requiring block size0x400 and selected flag0, and constructs its processing object at0x18000752c. The selected constructor is not itself a demonstrated primary-dataset/transfer-function reload. Its decompiler also omits some interface arguments; their exact ABI flow is not asserted from names.

The source0x18000d724 retains DSP/configuration interfaces and constructs parameter evaluators. Source processing0x18000d290 swaps source parameter records, handles reset flag+0x350, and invokes0x180010854 or0x18000f678 according to source mode. The reset operations shown clear evaluator buffers; no service-event wait or mapped dataset rebuild is shown there. Complete source consumer functions and source initialization are saved, alongside actual CPU factory, CPU initialization and processing constructor.

Consequently: an already-created stream has no proved event-triggered rebuild in this trace; a new engine can be created with the existing mapped-view interface despite InitOnce. Whether either reads changed transfer data through shared-memory aliasing, derives/copied filters earlier, or recomputes on a source-parameter change remains unresolved. The explicit-HRIR converter report proves service-side partition construction; it does not settle this downstream lifetime question.

All headless runs completed; edited Java and PowerShell scripts compiled/executed, selected functions were saved without output truncation, and source hashes were rechecked. No converter, reload operation or native acceptance test was implemented.

Remaining blockers: service0x18000e004's exact copy/extent/synchronization contract; RPC response's first-handle producer; primary mapped-data consumption by the source evaluators and CPU directional/filter-bank interfaces; transfer-data alias/copy lifetime and recalculation predicates; behavior if a published representation changes length/shape; current override/dataset type and observed live uptake/restoration. These require a separate bounded trace or an explicitly authorized native observation. A registry-path edit, stream reopen, or service-event function name alone is not a supported reload/restoration mechanism.
