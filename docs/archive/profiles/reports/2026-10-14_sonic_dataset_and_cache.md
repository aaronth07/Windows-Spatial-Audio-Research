> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

The installed Sonic-related data path is more concrete, but a system-wide custom Sonic listener profile remains unverified. This pass used inert copies and REA/Ghidra only. No registry value, endpoint, provider selection, service, driver or audio stream changed.

`HrtfDspCpu.dll` 2024.0.42, SHA-256 `4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9`, exports `CreateHrtfDataHelper` at0x180003170. The factory produces `HrtfDataHelper::vftable` at0x180141278. Its slot+0x18 is0x1800057e0, a dataset/environment conversion and serialization routine; slot+0x20 is0x180005c30, an environment metadata reader. Earlier HrtfApo evidence of a slot+0x20 call occurs in its environment-model loop, not its primary listener dataset. This distinction matters for the system Sonic linkage still to be proven.

## Dataset formats

The dataset conversion calls0x18000a370. That function recognizes two GUIDs and throws for an unknown GUID or insufficient length:

- `<identifier-removed>`: the observed DefaultHrtfs representation. Parser0x18000a190 reads four32-bit integers and three32-bit float scalars, then interleaved three-component positions, one auxiliary float per position, and two response arrays. Its final byte extent must match exactly. The conversion constructs explicit HRIR data after processing the arrays through0x18000e470/0x18000e090/0x18000df30. Complete response semantics and ear assignment remain unresolved; the serialized arrays must not be called direct HRIRs yet.
- `<identifier-removed>`: an explicit `Audio3D::HrirData_impl` representation. Constructor0x18000daac reads nonzero sample rate and filter count, a serialized position structure, then two `ImpulseResponseSet` blocks. Parser0x180010210 validates a nested GUID, header size24, nonzero response count, response-length list and total float-data extent. This establishes a native explicit-HRIR decoder, not complete delivery, interpolation or system-wide Sonic use.

The unchanged DefaultHrtfs.bin is4,227,116 bytes, SHA-256 `<private-artifact-digest>`. Its integers are48000 Hz,2048 positions,256 serialized response values per position,512 decoded response values per position. The three scalars are3.4890749454,0.7953904867 and0.0029154520; semantic names remain unresolved. Positions start at byte44 and are2048 distinct points, all within4.1e-8 of unit radius. The auxiliary vector starts at24620. The two float arrays start at32812 and2129964. All values are finite. The file is structured floating-point data; its previously uninterpreted bytes do not establish encryption.

The read-only inspector and results preserve unknown coordinate/ear/response semantics explicitly. Its rate/count/length/128-MiB bounds are conservative inspector limits, not asserted native limits. It refuses the explicit-HRIR GUID pending a separate complete metadata validator. Four new unittest groups cover the actual owned dataset/hash/offsets, invalid exact extents/GUIDs/dimensions, nonfinite metadata/arrays, CLI failures and input preservation; the three existing DTS groups also pass on Python3.8.

## Personalization, shared mappings and refresh

SSDM0x18000b380 loads HrtfDspCpu from the system search path, resolves CreateHrtfDataHelper and invokes slot+0x18 with the primary dataset, environment arrays and requested minimum frame count. The result is copied into separate primary and environment output vectors. Initial0x180006188 deliberately supplies a negative sentinel anthropometric value before invoking0x18000b954, forcing initial conversion. It constructs `HrtfSharedDataInstance`0x18000d4a8 from these converted vectors. Thus the registry-file loader reaches the inspected dataset conversion and shared-data publication; actual Windows Sonic DSP linkage still requires its engine factory/processing bridge.

On later requests0x180006d5c reads the registry files again but assigns the current shared instance's anthropometric value to the new temporary dataset object. It submits worker0x180002f60→0x180005398. The worker calls0x18000b954, which clears the changed flag and invokes conversion only if the provider's value differs beyond its floating-point tolerance. The worker publishes updated mappings through0x18000e068 only when that flag is set. Consequently changing dataset bytes/path alone is not demonstrated to invalidate this cache, and the selected branch indicates that an unchanged anthropometric value will skip conversion and publication despite rereading files. This is a static branch conclusion, not a live update experiment.

0x18000e068 copies converted primary/environment data into the shared mappings and signals an event. The global data-manager singleton is retained at0x180020a48. Destructor0x1800109e0 releases it; no independent supported desktop dataset-reload API has been established. No service restart, artificial anthropometric change, DLL unload, reboot or registry edit is proposed in this pass. Those must not be silently used as an uptake or restoration workaround.

## Evidence and next actions

Complete artifacts are HrtfDspCpu REA evidence and SSDM REA evidence, with matching snapshots. Key evidence IDs:

- Dataset recognition0x18000a370: `ev_4d9eac1f06cd6fa4965d598251d8e4f64f5e95ef15653e5aa13db97d1523d11e`.
- Default layout0x18000a190: `ev_b1ca73502da7bc98fb93e8c545b302c65f49f16c2e7c59b92d4f3bda64084ea4`.
- Explicit HRIR constructor0x18000daac: `ev_1db11dcc83bce114014bc929ec3ed0962b2f7976b8f0c5594421d58c16a31bce`.
- Nested response validator0x180010210: `ev_1e9e978e12c0cc118db773a50f4cd38cbfc1606e2162db05a340e8ab306e7eda`.
- Service conversion0x18000b380: `ev_364703c5d4c96fade20111f04397ced9e075096bdaec2f98452e73780e326f60`.
- Anthropometric change gate0x18000b954: `ev_3554e8f0b506c5cd3f1e0cc9353a24eda7fb1249c9039a5554f5d63282179786`.
- Later request0x180006d5c: `ev_23c3bdd99c324a484804a9a89543df438e3be05a1ea694e077f56b94ee9406b1`.
- Update worker0x180005398: `ev_71a3dc362f2b15bde77fa0ff262c8df998c2e834b43594f960a50487dc094c6e`.
- Mapping replacement0x18000e068: `ev_f93ccf67f42716d1d2d0bcf7bd34899d53311390204a1ba65aed6f828249bb08`.

Next: prove the desktop Windows Sonic engine uses the shared primary dataset; fully map the explicit-HRIR position/interpolation and ear/delay/gain metadata; find a supported cache refresh/restoration path. Only after those edges are evidenced should a SOFA converter or backed-up native import experiment be implemented. A dataset parser or successful registry write is insufficient proof of use.

Further factory tracing retained CreateHrtfEngineFactory at0x18000c3b0 and HrtfEngine initialization0x18000b9e0. The latter initializes a global configuration through0x18000b170, loads HrtfDspCpu, resolves CreateHrtfDspFactory, and calls its slot+0x18 before constructing the engine instances. Complete evidence is in the updated HrtfApo bundle, including `ev_cc515aa382e460eb1acac928f6dd557b3438276fc933390900e4e1ebfbc6ed37` and `ev_bdb7a27fe7acca3b16a10964cd1221b8871c717c2b9ac3d5e52ed5bb28299836`. Resolve callback0x18000b170 and the CPU factory's data arguments next. There are distinct HrtfEngineFactory vtables at0x180014f18 and0x180014f68; the former has this-adjusting/forwarding methods, so method names must not be assigned by assuming it is the primary engine interface. The audioeng.dll loader/caller is still untraced. The shared REA target was closed with its matching snapshot.
