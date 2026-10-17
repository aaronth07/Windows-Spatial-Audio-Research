> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Native PHRTF spatial-toggle source review

The four-run OFF/ON test is ready for prior coordinator's source review. It has not been executed. The new runtime release file is absent; the old six-run approval cannot release this sequence. The original runner, native executable and property transaction remain unchanged.

The runner is `run_native_lifecycle_validation.ps1`. Its exact changes from the successful six-run harness are in `native_lifecycle_runner_20261013.diff`. `native_lifecycle_source_review_20261013.json` lists the five source/executable hashes with `authorized: false`. The new comparator is `compare_native_lifecycle_captures.py`.

The sequence is `baseline candidate_before_toggle candidate_after_toggle restored`. Initial state requires Access closed, approved console/multimedia playback defaults, Atmos OFF, original API14 property and DAFM VT_EMPTY. A fresh scoped backup is verified before enabling Dolby. The existing input is one dynamic object with five fixed directions, ten seconds, mono float32 48 kHz, five impulse samples at 0.01. Capture uses the existing paired endpoint. Persisted playback/recording formats remain unchanged from the initial inventory; the latest verified formats were stereo 48 kHz PCM16/PCM24. There is no format setter in this runner.

After the baseline capture, the unchanged C# transaction sets and commits only `{1b4dab55-b1fb-4d8c-8317-f2d4a96efbb8},1` in the normal consumer Dolby context on selected virtual endpoint. API62791 and REG62799 candidate readbacks must match their pinned hashes. Other User values and Access files must match fresh backups. The callback captures the candidate, disables Atmos, verifies the same candidate while OFF, enables Atmos, verifies it again, and captures the candidate after the toggle. It never rewrites the candidate to repair a mismatch. Both BATs have exact path/hash checks and immediate console/multimedia default guards.

The C# inner finally restores and commits the original property before default-endpoint verification, including when the callback fails. Restoration is checked by exact API/registry bytes and all six User values. The restored capture follows only successful restoration. The PowerShell outer finally independently checks the fixed context/app backups, attempts guarded Atmos OFF, and records final baseline/OFF/default/format state. A default mismatch blocks the BAT and records incomplete cleanup. It does not redirect cleanup to another endpoint. Access remains closed; cache/hive writes and leaving the profile installed are outside this test.

The requested exclusive window is three minutes. Four captures usually take about 44 seconds; each native child has a 20-second limit and each BAT child a 15-second limit. The inherited read-only getter/COM operations have no independent hard deadline, so a three-minute whole-transaction bound is not guaranteed if those operations hang. The controller is never force-killed to impose a whole-transaction deadline because that could skip property restoration. Previously observed getters and the successful six-run transaction were prompt; this limitation remains explicit for release review.

The new `Read-State` verifies the exact endpoint and successful activation/open/get statuses, property ID, VT_BLOB type, expected payload length/hash, normal spatial subtype and DAFM VT_EMPTY at every checkpoint. Candidate context verification substitutes only the normal target registry value, retaining the fresh baseline expectation for every other value. Default inventory and persisted format drift abort the sequence.

Offline validation completed:

- PowerShell AST parsing and local C# structure/input validation passed; baseline14, candidate62791, native variant24 bytes. No COM activation occurs in local mode.
- The inherited default/cleanup suite passed 14 mocked cases against the new runner: ten blocked wrong/missing/duplicate default cases, two allowed mock BAT invocations, initial preflight and cleanup verification independent of the default.
- Extracted state/callback tests passed 26 cases, including candidate/baseline while ON/OFF; wrong hash, type, size, property ID, getter status or endpoint; duplicate values; DAFM presence; Access launch; format drift; exact callback order; and aborts before the second candidate capture on OFF/readback/ON failures.
- The comparator passed synthetic retained/lost-effect, shared-delay, restoration-mismatch, silence, short, mono and nonfinite checks. Six CLI cases verified complete synthetic WAV input and rejected bad order/hash, failed operation and unverified/incomplete restoration.
- Invoking the runtime switch without the new release file exited before operation-directory creation. Historical getter files were unchanged. No live getter, BAT, stream or property API was called.

Evidence files are `native_lifecycle_default_guard_tests_20261013.json`, `native_lifecycle_state_tests_20261013.json`, `native_lifecycle_comparator_tests_20261013.json` and `native_lifecycle_release_gate_test_20261013.json` at the project root. Tests use mocked launchers/readers and synthetic WAVs; they do not prove a real OFF/ON lifecycle or injected COM-failure recovery. The unchanged C# restore ordering was separately reviewed and its successful actual restoration is documented in the prior native-load transaction.

The four-capture comparator uses a single shared stereo alignment, preserving ear level/timing. Every baseline/restored response must agree within relative L2 1e-6; candidate changes must exceed max(5 times restored error, 1e-6), and candidate before/after must agree within that threshold. Inconsistent controls are inconclusive. Passing this test would establish processing across the spatial toggle only; Access launch, reboot and user-facing cache acceptance remain open.

After an exact release and exclusive-window instruction, the commands would be:

Historical workspace command omitted; use the public [example guide](../../../../examples/README.md).

prior coordinator must review the exact five hashes before a new runtime release record is created. No release record was created for this preparation.
