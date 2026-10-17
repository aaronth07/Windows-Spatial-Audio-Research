> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Ear-output routing review

Review scope: the routing report; native function/instruction dumps explicitly cited by it; retained artifact manifests and hashes. This was a static evidence review only. No DLL was loaded or executed, and no audio or renderer state was accessed.

## Findings

- The listed function addresses are consistent with the linked dumps: metadata evaluator 0x18003e550 appears in DolbyCoefficientResearch/native_direction_functions.txt; direct accumulator 0x1800420d0 appears in DolbyCoefficientResearch/native_coefficient_functions.txt; wrapper 0x180025a80 and calls to both appear in DolbyCoefficientResearch/native_filterbank_pivots.txt. Those excerpts support the separate field order and accumulator output arguments. They do not establish physical left/right ear identity.
- Crossfade 0x18011bd40 is present in DolbyCoefficientResearch/native_ear_output_functions.txt. Its body contains the dolby::oar::CrossfadeModule::Process diagnostic string, fade modes, and a loop applying a common scalar to channel-strided samples through the descriptor pointer at +0x18. This supports the reportâ€™s limited channel-order/common-per-sample-gain statement.
- The previously missing supplemental evidence is present at the exact path <local-path>
- The native_path_consumer_instructions.txt listing is 60,585 bytes and its SHA-256 matches the native_path_artifact_manifest.json entry. The manifest identifies the limitation that the EncodeAudioData decompile excludes the computed-switch path and that recovered assembly is retained; this is consistent with using the instruction listing for the call path.
- The documented synthesis/alias and Windows output-channel bridge remains outside the specific excerpts checked here. The consumer call and Crossfade evidence do not establish that complete correspondence.

## Hash checks

- Current <local-path>
- Both retained inspection.json input hashes were recomputed from their recorded paths and matched: owned profile <private-artifact-digest>; candidate API payload <private-artifact-digest>.

The audit correction resolves the cited consumer-call evidence and independently verifies the DLL and instruction-dump hashes against their manifest. No substantive routing findings were edited.
