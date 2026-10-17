> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Settings authorization log — 2026-10-13

## Authorization recorded

<identity-removed> authorized project-related Dolby Access settings changes within the documented test, backup, endpoint, and output boundaries. Research may perform already-authorized, backed-up operations when actual access permits; no elevation or permission bypass is authorized. <identity-removed> separately approved the exact endpoint-property set/readback/restore test. A prior reviewer reviewed the operation and approved proceeding after the required preflight, with Dolby Access closed, Atmos off, and no audio or spatial enablement. Other registry values require separate exact approval; the full registry reference or backup must never be imported.

## Current status

- <identity-removed>'s exact property-only set/readback/restore authorization remains in force. A prior reviewer reviewed and approved proceeding under the stated preflight and stop conditions; see dot_property_test_review_20261013.json.
- The round trip passed at 2026-10-13T13:27:59Z. A fresh backup with backup_complete=true was recorded immediately before the test in private/native_property_test_20261013T132759546Z. Dolby Access process count was 0; active/default spatial GUIDs were zero.
- The candidate API value (62,791 bytes; SHA-256 <private-artifact-digest>) was set, committed, and read back. It was then restored to the prior 14-byte API payload (SHA-256 <private-artifact-digest>). The 22-byte registry representation was restored to SHA-256 <private-artifact-digest>. All SetValue/Commit calls returned S_OK.
- Post-restore verification confirmed all six User-context values, app files, defaults, and Atmos-off / zero active-default spatial state. The operation used no audio stream or spatial command and did not launch Dolby Access. Evidence: property_round_trip_20261013.json.
- This establishes writable property persistence and immediate baseline restoration only. Native load, personalized audio processing, Dolby Access UI integration, and persistence across app launch/restart are unproven. No further live operation is implied by this test.
- The 12:45 incomplete backup and 13:22 assessment's “paused pending prior coordinator's evidence review” language are historical. The complete 13:06 backup preceded review; the exact test was later reviewed and completed at 13:27. Exact approval is still required for any different registry key/value. Never import the full registry reference or backup.

## Stated boundaries

Do not change default output, routing, or Vivaldi settings. Do not use KS Repeater or bridges, route to physical speakers, or change drivers, services, binaries, licenses, accounts, or pro Dolby Atmos Renderer state. Before any live PHRTF test, record the exact action, current values, backup location and hash, rollback steps, and post-restore verification. The exact endpoint property set/restore is approved as recorded above; any other registry key/value still requires separate approval. Never import the full registry reference or backup.




