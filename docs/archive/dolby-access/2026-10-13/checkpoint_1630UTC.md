> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Checkpoint — 2026-10-13 16:30 UTC

The six-run native Spatial Sound transaction remains the only executed native capture sequence; its candidate result and verified property restoration are unchanged. The separate four-run OFF/ON sequence remains unexecuted, and its runtime release record is absent. prior coordinator is reviewing the revised sources.

The getter-deadline runner is SHA-256 `<private-artifact-digest>`, revised from the previously reviewed runner hash `<private-artifact-digest>`. Comparator hash remains `<private-artifact-digest>`. The C++, C# and native executable are unchanged.

Twelve focused tests passed. Three reader cases exercise the configured 10-second wait, output preservation, and timeout termination; one tests nonzero exit; one checks the reader allowlist; three inject timeout in the actual candidate callback and reach mocked property restoration plus the runner's actual outer-finally cleanup; four benign fixture processes verify Windows process transport and nonzero exit handling. Wait timeout and property transaction restoration are mocked. The actual C# restore has source review and the earlier six-run successful transaction as evidence, but no real COM failure was injected. No real getter, BAT, audio, or property API was invoked.

The owned getter process is limited to 10 seconds, followed by a 5-second termination wait. Controller-side COM calls and the pre-mutation backup preparation subprocess remain unbounded, so the three-minute whole-transaction duration has no hard guarantee. Both candidate captures in the proposed sequence use Dolby ON; the OFF checkpoint only reads the property.

See `native_lifecycle_getter_review_1628UTC.md`, `native_lifecycle_getter_review_20261013.json`, and `native_lifecycle_getter_timeout_tests_20261013.json`. Historical runner hashes remain preserved in their dated records. No source, package, or live system state was changed for this report update.
