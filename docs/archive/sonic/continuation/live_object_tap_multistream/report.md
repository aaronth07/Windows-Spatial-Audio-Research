> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Multiple Windows Spatial Sound streams and object reuse

Two actual Windows Spatial Sound streams passed capture through the separately compiled ordinary producer DLL. Each stream supplied two objects for one second and replaced object slot 0 after EOS and release at the midpoint. The replacement received a new monotonic object ID. All 192,000 float samples and all metadata matched the producer reference exactly. The one live trial took 1.103713 seconds; no retry was needed.

The earlier `live_object_tap` source and results are preserved. This directory contains the separate implementation, binaries, logs, capture carrier, offline checks and manifest. Only the producer DLL's CoCreateInstance import was temporarily patched in the current process, then restored. Source inspection found no endpoint/default/provider/configuration/registry/service changes.

| Observation | Result |
|---|---:|
| Actual streams / objects active per stream | 2 / 2 |
| Format / actual update size | 48 kHz float32 mono / 480 frames |
| Actual Windows capacity reported | 128 dynamic objects |
| Source updates / final EOS drain updates per stream | 100 / 1 |
| Valid frames for each of four source slots | 48,000 |
| Object replacements per stream | 1 |
| Globally distinct object lifetimes | 6 |
| Captured records / float samples | 400 / 192,000 |
| Actual forwarded Begin / End calls | 202 / 202 |
| Copies before real End / buffer pointers cleared after End | 400 / 400 |
| Final inactive objects per stream | 2 |
| Stop / Reset on both streams | S_OK / S_OK |
| All proxy references released / producer import restored | True / true |
| Unknown interface passthroughs exercised | 0 |
| Host instrumented operator new/delete calls during updates | 0 / 0 |

Both streams were activated and started before entering the update loop. The ordinary producer waits on both actual Windows event handles and services the signaled stream, with one MTA thread interleaving calls. Their rendering intervals overlap. This trial did not use separate concurrent COM worker threads.

## Object lifecycle and carrier

Each `StreamProxy` points to its own `StreamState`: four preallocated object wrappers, update/frame counters, capture records, float storage and capture statistics. No append index or PCM storage is shared between streams. Atomics assign global stream and object creation IDs. IDs are independent of object pointer values, source indices and pool slots. COM reference counts retain their normal interlocked operations.

The observed object lifetimes were:

| Stream ID | Source slot | First lifetime ID | Replacement ID |
|---|---:|---:|---:|
| 1 | 0 | 1 | 5 |
| 1 | 1 | 2 | No replacement |
| 2 | 0 | 3 | 6 |
| 2 | 1 | 4 | No replacement |

At frame 24,000 the producer marks source slot 0 EOS. On the next update it checks the actual object's IsActive result, releases that interface after it becomes inactive, and activates another actual dynamic object. The wrapper pool reuses its slot while assigning a new object ID. The other source continues without EOS at the midpoint. Both final objects must become inactive after EOS; failure to do so within four drain updates fails the trial. Stop and Reset must also succeed.

`live_run/capture.json` contains `rate`, `coordinate_system` and records. `capture.f32` is little-endian float32 mono data concatenated in record order. The final merge groups records by stream, then per-stream update and source slot; it happens after streaming ends. No global presentation clock or cross-stream hardware timestamp is supplied. The record contract is:

| Field | Meaning |
|---|---|
| `stream_id` | Positive monotonic ID assigned at wrapped stream creation |
| `object_id` | Positive globally monotonic ID assigned at each wrapped object creation |
| `update` | One-based successful Begin ordinal within that stream |
| `source_index` | Object pool slot; reuse does not preserve object identity |
| `generation` | Zero-based reuse count of the local pool slot, retained as diagnostic metadata |
| `frames` | Full actual Windows object buffer extent in frames |
| `valid_frames` | EOS valid frames, otherwise frames |
| `eos` | Successfully forwarded SetEndOfStream for this lifetime |
| `type` | AudioObjectType; Dynamic is 1 |
| `position` | Windows X right, Y up, Z backward |
| `volume` | Successfully forwarded object volume, applied separately from PCM |
| `sample_offset` | Float offset into the concatenated capture.f32 |
| `forward_end_hresult` | Result of the subsequent actual EndUpdatingAudioObjects |

For SOFA coordinates use `(-WindowsZ, -WindowsX, WindowsY)`. Apply each record's volume once. Raw PCM is approximately ±0.01; volumes are 0.0005 and 0.00075. These values do not measure endpoint output or acoustic level. `reference.json/.f32` is recorded independently in the producer DLL; its object IDs remain zero. Its stream number is a logical creation ordinal. The test maps that ordinal to stream IDs through the producer's explicit sequential stream-creation contract, not pointer or PCM identity inference.

The buffers are copied before forwarding actual EndUpdatingAudioObjects, then pointers are cleared. Windows invalidates GetBuffer pointers after End. Microsoft interface documentation (historical reference unavailable)

## Validation and review

The final x64 MSVC build passed C++17, `/O2 /MT /W4 /WX`; `build_second_review.log` records it. Before the live trial, 19 native offline cases passed: capacity/overflow/null inputs, concurrent independent stream-log writes, globally unique IDs allocated across two threads, producer-only import mutation, page-protection preservation, explicit/destructor restoration and missing-import rejection. These concurrent offline log and ID tests do not establish full concurrent native COM stream handling.

`offline_second_review/result.json` adds ten cases. It independently reconstructs all source PCM, position and volume formulas; verifies EOS/reuse boundaries and each lifetime's stable ID; and rejects cross-stream metadata, reuse of an old lifetime ID, invalid offsets/EOS extents, PCM corruption and truncation. Existing output, missing/invalid DLL and missing arguments fail. Capture-file hashes remain unchanged. The offline verifier starts no spatial audio stream.

The second code pass inspected per-stream state ownership, generation reset, new object IDs on reuse, COM cleanup on every failure path, EOS completion, capture-before-End ordering, merge offsets, import rollback, numerical reference checks and allocation-counter scope. It also corrected the host's unknown-interface aggregation to include object passthrough counts from each stream before the live build. The previous implementation's global callback state is now confined to each stream. Capture and reference storage is bounded at 1,024 records and 131,072 floats per stream, allocated before streaming; files are written afterward.

The zero allocation result counts only this host's instrumented C++ operator new/delete between successful Begin and completion of End. Windows DLLs, the producer's separately linked runtime, malloc and HeapAlloc are outside that counter. Update CPU distributions and scheduler deadlines were not measured here.

## Boundaries and reproduction

This host reserves two stream states for two stream creations during one run, with four object wrapper slots per stream. The tested control and COM lifecycle calls run on one thread. Full concurrent control-wrapper statistics/lifetime handling, stream-state recycling after release, arbitrary interface coverage and arbitrary game compatibility remain unverified. Unknown QueryInterface requests delegate to the real interface and are counted; an exercised passthrough could bypass capture, so zero such requests is required for this result.

Endpoint selection enumerates active render endpoints read-only, requires exactly one selected virtual endpoint match, and passes its explicit endpoint ID to the producer. Unsupported formats, missing/ambiguous endpoint, object/stream activation failure and event timeout fail without changing configuration. This actual source/metadata capture does not include the Windows renderer's output PCM and does not establish physical crosstalk cancellation.

From this directory, prerequisites are Windows x64, Visual Studio C++ tools, a Windows SDK and Python 3.8 or later. Python checks need only the standard library. Every output directory must be new.

Historical workspace command omitted; use the public [example guide](../../../../../examples/README.md).
