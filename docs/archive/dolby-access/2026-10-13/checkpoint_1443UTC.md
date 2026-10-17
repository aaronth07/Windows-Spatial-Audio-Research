> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Checkpoint — 2026-10-13 14:43 UTC

## Default-device guard correction

Dot's correction is recorded in native_default_guard_1440UTC.md with exact source diff native_default_guard_1440UTC.diff. The PowerShell runner now checks both console and multimedia defaults against the approved selected virtual endpoint endpoint in the initial preflight and again immediately before each enable/disable BAT. It records phase and observed role/ID pairs. A wrong, missing, duplicate, or unreadable default blocks process launch and records incomplete cleanup.

The C# wrapper's fixed-property restoration remains before the default/OFF guard. Outer fixed-context and app-file backup verification is independent of OFF cleanup, so a changed default does not cause the BAT to target another endpoint or skip backup verification. BAT paths and hashes remain restricted to the two selected files; no alternate writer or retry was added.

All 14 focused tests passed: 10 blocked BAT cases, two allowed mock cases, an initial-preflight guard case, and a wrong-default cleanup record with fixed-backup verification reached. These use mocked MMDevice and process-launch functions. Separate wrapper checks exercised exit-code propagation and termination/disposal using a 150-ms timeout on an owned silent child. The configured 20-second native and 15-second BAT limits and actual BAT behavior remain untested. No real MMDevice query, BAT, property API, or audio stream was used by these tests.

## Host and preparation evidence

The runtime runner hash is now <private-artifact-digest>; the other four pinned artifact hashes are unchanged. The C# layout/hash/JSON checks passed on PowerShell 7.6.5 and .NET 10.0.11. A separate Process.Kill(true) test terminated an owned parent and child while preserving an unrelated witness; test processes were disposed/stopped. This validates the host API behavior, not real BAT or audio handling.

The six-capture plan and comparison remain under Dot review. Native runtime remains unreleased: the preparation record says no streams opened and no spatial BATs run. Actual MMDevice behavior, BAT compatibility, real stream failure restoration, native coefficient load, and candidate-specific output remain untested. No broad build or unrelated process action was performed.

Evidence: native_default_guard_tests_20261013.json, native_host_compatibility_20261013.json, native_load_preparation_20261013.json, native_default_guard_1440UTC.md, and native_default_guard_1440UTC.diff. The primary-authored native_load_validation_plan_1355UTC.md and source/evidence files were not edited in this reporting task.

