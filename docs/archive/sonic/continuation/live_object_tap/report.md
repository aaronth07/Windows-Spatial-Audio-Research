> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Forwarded Windows Spatial Sound object capture

The native tap captured an ordinary, separately compiled producer DLL's two real Windows Spatial Sound objects. The producer used the actual selected virtual endpoint endpoint, client, event-driven render stream and object buffers. The tap copied all 96,000 float samples and every position, volume, frame count and EOS value exactly before forwarding the final update call. The current source and binaries passed `live_second_review/result.json` on 2026-10-16.

The producer's `CoCreateInstance` import was the only import changed. Its original pointer and page protection were restored; the host's import remained unchanged in the offline restoration test. The source contains no device/provider/default/registry/service configuration changes. All generated files are inside this directory.

| Observation | Final live run |
|---|---:|
| Format | 48 kHz, mono float32 per object |
| Actual Windows update size | 480 frames |
| Reported maximum dynamic objects | 128 |
| Source updates / EOS drain updates | 100 / 1 |
| Valid frames per source | 48,000 |
| Captured records / float samples | 200 / 96,000 |
| Actual forwarded Begin / End calls | 101 / 101 |
| Objects inactive after EOS | 2 |
| Metadata and PCM reference comparison | Exact |
| Captures before forwarding End / cleared pointers after End | 200 / 200 |
| Unknown interface passthroughs exercised | 0 |
| Proxy references released / import restored | Both true |
| Stop / Reset | S_OK / S_OK |
| Elapsed time | 1.082667 seconds |

The maximum raw PCM amplitude was 0.009999847. The producer applied object volumes 0.0005 and 0.00075. These are source values and metadata, not a measurement of the endpoint's rendered output or acoustic level. No audio playback beyond the two bounded one-second producer trials was performed. The second trial validated the stricter EOS completion condition found during review. Both trials produced capture PCM SHA256 `<private-artifact-digest>`.

## Capture interface

`capture.json` supplies `rate`, `coordinate_system` and ordered `records`. `capture.f32` stores little-endian mono float32, concatenated in record order. `sample_offset` is a float index, not a byte offset. Each record contains the full actual Windows update extent, including zero padding after a partial EOS. The final observed transaction had 480 valid frames and required no padding.

| Field | Meaning |
|---|---|
| `update` | One-based successful Begin transaction ordinal |
| `id` | Monotonic positive ID allocated at wrapper creation, stable until release |
| `source_index` | Fixed object pool slot for this bounded producer, 0 or 1 |
| `frames` | Actual Windows update frames copied from GetBuffer |
| `valid_frames` | EOS frame count when EOS is set, otherwise frames |
| `eos` | Producer successfully called SetEndOfStream in this object lifecycle |
| `type` | Actual requested AudioObjectType; 1 is Dynamic |
| `position` | Windows X right, Y up, Z backward |
| `volume` | Successfully forwarded object volume |
| `sample_offset` | First float in capture.f32 |
| `forward_end_hresult` | HRESULT from the subsequent real EndUpdatingAudioObjects |

Records are ordered by update then pool slot. The source position changes once per update. For the root SOFA/VBAP pipeline use SOFA XYZ = `(-WindowsZ, -WindowsX, WindowsY)`, retain the transaction boundary, and apply the recorded object volume exactly once. The raw PCM has not been multiplied by the volume. `reference.json` and `reference.f32` are recorded by the ordinary producer; reference IDs remain zero because the producer has no knowledge of the capture wrappers.

Windows invalidates object-buffer pointers after EndUpdatingAudioObjects. `StreamProxy::EndUpdatingAudioObjects` therefore copies the buffer first, calls the actual Windows interface, records its HRESULT, and clears all retained pointers. Microsoft interface documentation (historical reference unavailable)

## Implementation and scope

`producer.cpp` has no wrapper, hook or capture-ID knowledge. It uses documented SDK COM calls with an explicit endpoint ID. The host resolves exactly one active render endpoint whose friendly name contains selected virtual endpoint; ambiguity or a missing endpoint fails before starting a stream. Unsupported float48k, unavailable object capacity, activation errors and event timeout fail without changing configuration. Endpoint enumeration is read-only. The control sequence follows the Microsoft spatial object rendering example (historical reference unavailable)

`tap_host.cpp` delegates the tested enumerator, device, spatial client, render stream and object interface methods to their actual Windows implementations. The producer's import is patched in its own loaded image only. All capture records and sample storage are allocated before streaming. Four object wrappers are preallocated; this proof uses one stream, one calling thread and two objects held until EOS and release. Captures are written to disk after streaming ends. Output directories and files reject overwrite.

The host counted zero calls to its instrumented C++ operator new/delete between successful Begin and the end of End. This establishes the host wrapper/capture path's observed behavior, not allocation behavior inside the producer's separately linked runtime, Windows DLLs, malloc or HeapAlloc. Capture storage is bounded at 1,024 records and 131,072 floats. No callback scheduling or CPU-duration distribution was measured here.

Unknown QueryInterface requests pass through to the underlying object and are counted. Such a returned interface could bypass capture and breaks a generic proxy identity claim; none occurred in this producer. There is no game injection, arbitrary application compatibility, multi-stream synchronization, object reuse stress proof, audio endpoint output capture, or acoustic cancellation proof in this result. The live stream is an actual Windows source/metadata capture route that the root worker can feed into the independently tested offline rendering chain.

## Review and verification

MSVC x64 `/std:c++17 /O2 /MT /W4 /WX` builds the producer DLL, capture host and offline tests. `build_second_review.log` records the final build. A second code review checked COM ownership, failure cleanup, buffer expiry, object lifecycle, bounded capture extents, callback allocation scope, import rollback, output overwrite behavior and carrier ordering. It corrected the EOS completion hole: the producer now fails if either object remains active after four drain updates, and the host independently requires both objects inactive plus successful Stop and Reset.

`offline_second_review/result.json` passed 16 native boundary/restoration cases and nine additional cases. These cover exact sample capacity, exhausted record/sample capacity, invalid uint32 extents, null inputs, producer import isolation, page protections, explicit/destructor restoration, missing CoCreate import, existing output refusal, missing/invalid DLL, missing arguments, malformed object IDs, invalid offsets/EOS extent, PCM corruption and truncation. The offline verifier independently reconstructs the source PCM sequence and position/volume formulas. It verifies capture/reference files are unchanged and starts no spatial stream. Native failure diagnostics are retained beside that result.

Reproduce from this folder with Windows x64, Visual Studio C++ tools, Windows SDK headers/libraries and Python 3.8 or later. The verifier needs only Python's standard library. Use new output names for every run.

Historical workspace command omitted; use the public [example guide](../../../../../examples/README.md).

The current result is specific to this separately compiled producer and the selected endpoint at test time. It proves source PCM/metadata capture while forwarding to Windows; extending the import/proxy coverage to an actual game remains a separate integration task.
