> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Native evidence locations

All AudioEng addresses are preferred-base VAs at 0x180000000; audiodg addresses use 0x140000000. The JSON exports preserve full function bodies, tool limitations, binary hashes, analysis profile and Evidence IDs. The adjacent `_index.json` files locate records without opening the complete export. These are static observations, except where an owned native probe separately executes the route.

| Claim | Function/evidence |
|---|---|
| Provider selection and recognized format GUIDs | AudioEng71754, ev_b4be8225fca7706ab7d68ee161234e08e83e5a8be26c807e2bc4d0a908eccbce |
| Optional provider Initialize | AudioEng71370, ev_55fdcfefe7085810630f92a7fa6c48a2868f64651459ed95449cfe71bcf46bbc |
| Dynamic objects and metadata submitted before output | AudioEngf83b0, ev_d886a0e77a082e52ba1b7902e03fbc1ee308b9533ca02ce67ec8c0417cf4bd06 |
| 16-channel local format admission | AudioEng70180,136d0,eaa0; format_admission_probe.py |
| Descriptor byte preservation/frame clamp/type reduction | AudioEng2a24c, ev_c05dceccd02a746b77fac1a49e12d4bafc17ba95ede72ac61cfce30ee7a8e773 |
| Numeric object key use | AudioEngf7044, ev_0ee3cdf3381fedf9da9724e075e9cced8c9e0f1a96db4ce6b8a9ee3f96ad34a9 |
| Sonic engine construction during lock | AudioEnga790, ev_ab6fd47d10c502470753553a91e8d95a030b98dc3bbb16d45618222e9d8a7185 |
| Native HRTF initialization and CreateEngine | AudioEng530d8,537f8; ev_45ca1e53f7443f4152fb1b620c43d853d3a32bbdd5fa6c93643bd632f3cd4c06 and ev_0b79215640f6e9ae2fd2752fd89ae482f33051654408fd7838d893b4d04c66d3 |
| Effect construction/InitializeSystemEffect in pipe format stage | audiodg11698, ev_48892418d2801e6c0a2feec008852a0c9110bf1ef1a7672553a21e97dd90d864 |
| Actual effect Initialize call | audiodg33cf8, ev_2b5c696031083e50a92b7190f6a2c65e50121d89a897d282c358ab92249e8c4e |
| Construction fed8 before ConnectAPOs4878 | audiodg5f80, ev_359d745c97cfaa4578ebd9baae8ebde664707eee3a582074db12b57f4bd5ed0c |
| ConnectAPOs calls ConfigureAPOInterfaces | audiodg4878,52c4; ev_4e73e235a04e0f338040cc5d120ddc0e3694f64e82328fe65ae2d73721f442bf and ev_b52bdf382c46613e2ad345eddf169bc360746a681f00c09326420b36f5c35cb3 |

HrtfApo minimum-frame getter assembly is in quantum_getter_assembly.txt. Existing full Initialize and source initialization exports are at workspace SonicRendererResearch/apo_initialize.json and apo_sourceinit.json; processing is apo_fn_0x18000b310.json. The getter sets the engine quantum before factory activation. The API-set availability branch and its optional GetHrtfMinFrameCount call must not be erased from the interpretation.

The audiodg file is a read-only copy, SHA-256 <private-artifact-digest>. It was only statically analyzed, never launched or attached to. Indirect graph interfaces and endpoint-specific composition remain unresolved; per-pipe construction sequence does not establish process-wide first-load order.

The original first zero-GUID attempt lacked CreateHrtfDataHelper and faulted at engine creation. The corrected module exports both CPU entry points and forwards the helper to the pinned native DLL. Keep this failure diagnosis separate from unsupported or impossible route conclusions.

All runtime probes pin the native input DLLs before calling private addresses. Their hard-coded RVAs are intentional build-specific research dependencies, not a version-independent integration ABI. Native callback data, descriptors and metadata pointers are borrowed for that call; no asynchronous retention of those pointers is proven safe.
