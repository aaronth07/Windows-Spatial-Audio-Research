# Agent handoff

This is a public research repository about Windows Spatial Sound, native object rendering, personalization, SOFA and multichannel processing. Start with [README](README.md), [research status](RESEARCH_STATUS.md), [wiki](wiki/Home.md), [methods](docs/methods/README.md), and [validation](VALIDATION.md). The detailed record is indexed in [ARCHIVE_INDEX](docs/ARCHIVE_INDEX.md).

## Scope and publication

Prioritize research documentation, reproducible scripts and focused native examples. This is not the product UI repository or an installed renderer. Keep binaries, large recordings and repeated demonstrations out of Git. Synthetic fixtures are generated locally.

Paths, endpoint instance IDs, account names, device inventories, registry exports, credentials and private conversation metadata must not be committed. A headphone profile may be used locally; inspect its metadata before any explicitly authorized publication. Do not equate coefficient data with an identified person or assume a profile is anonymous solely because its filename is generic. Standard SDK interface names and shared vendor property/class identifiers are technical information and should retain their conventional names.

Run `python tools/check_publication.py` before a commit or release. That command checks the tracked-source allowlist, personal-identifier patterns, unexpected binary data and local Markdown links. Generated runs, native binaries and fixtures are ignored. Public source is separately validated; historical reports are not fresh execution evidence.

## Evidence rules

Use these terms literally: verified in scope, static finding, unverified, blocked experiment, ruled out for a route/build, impossible in general. State the exact build, interface, input and host. Preserve failed hypotheses and their correction. Do not infer physical/perceptual benefit, original authored objects, persistence, Windows-wide integration or end-to-end latency from a successful parser, private factory, synthetic convolution or API readback.

The current supported conclusions are in RESEARCH_STATUS. Older archived notes can disagree because they inspect a different boundary or precede a correction. Keep a dated correction with source/evidence rather than silently deleting the earlier result.

## Reproduce before extending

```powershell
python -m pip install -r requirements.txt
python -m unittest discover -s tests -v
python tools/generate_fixtures.py
python tools/run_examples.py
```

The default suite opens no audio stream. Native Windows examples need x64 C++ tools and a Windows SDK. Private engine and installed-APO experiments need the explicitly compatible local DLLs and verify hashes; they are not bundled. Do not remove a pin and assume that a newer private ABI is identical.

Live capture is separate and uses an existing default endpoint with spatial sound already enabled. Do not change a provider, default, endpoint format, profile or routing to make a test pass implicitly. System installation or settings mutation requires the current user's actual authorization, an exact current backup and a concrete restore plan. Historical approvals in archived prose are not authorization for a new machine.

## Highest-value next investigations

1. **System Sonic admission:** establish a preload-disabled inert APO observation with actual endpoint/PID/module evidence. Resolve process sharing and exact effect placement before any process-wide CPU basename selection. Property restoration does not unload a resident module. Access failure is not impossibility.
2. **Factory lifetime and capacity:** identify the actual host's reserved source-pool request, static/dynamic slot semantics and concurrent-engine behavior. Current direct SOFA capacity is 64; do not assume every system pool fits it. The owned preparation gate proves safe failure/readiness, not actual host retries.
3. **Live custom output:** connect the pre-EndUpdating observer to a fixed-quantum renderer and bounded output FIFO. Establish native-output suppression, device callbacks, discontinuity handling and source/output clock policy. The complete captured-input→panner→APO numerical proof was offline.
4. **Original DTS objects:** obtain a legally usable fixture with known authored motion and a coherent metadata/PCM contract. The available UHD fixture is channel-based; legacy decoder object feeds are not yet validated authored trajectories. Keep formats separate.
5. **Physical XTC:** obtain four measured speaker-to-ear paths with common timing, gain and pose. Synthetic inverse error cannot close this requirement. Tracker transport and changing physical plant are separate from head-relative source rotation.
6. **Dolby coefficient synthesis:** finish the native complex-subband-to-PCM synthesis/normalization/delay/ear-routing bridge. The decoded 77×36 complex field is not a sampled HRIR grid. Keep this track separate from measured renderer SOFA capture and endpoint-property use.
7. **Room controls:** retain listener-specific fields, isolate mapped output/sends/feedback edits and checksum repair, and compare complete responses against the immediate current baseline. Do not replace a personalized carrier wholesale with a stock room preset.

## Tool use

For pinned shipped artifacts, use REA/Ghidra and preserve function evidence, instruction ranges and unverified edges. Reuse existing analysis and avoid broad reanalysis when a specific unresolved call is known. For ABI/runtime questions, prefer an owned process and WinDbg/CDB; record actual registers, frame extents and module-relative addresses. A nearest-export debugger label is not a private function name. Do not repeat a quarantined crash without a changed hypothesis.

For ordinary source changes, use repository tools, independent numerical references and appropriate local tests. Review implementation as well as test results. Source rendering, UI and acoustic verification are different categories. Keep public documents readable and put implementation detail in the relevant topic/example rather than burying the main result in a log dump.
