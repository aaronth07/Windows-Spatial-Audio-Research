> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Checkpoint — 2026-10-13 13:28 UTC

## Result

prior coordinator reviewed the previously approved exact endpoint-property-only test. At 13:27:59 UTC the candidate API payload was written, committed, and read back; the original baseline was then written back, committed, and verified. All API activation/open/set/commit and restore set/commit HRESULTs were S_OK. The exact baseline was restored.

Target:
- Endpoint: {<identifier-removed>}
- Context: 45da5c30-2837-4ac4-b1e2-50acc3865974
- Property: {1b4dab55-b1fb-4d8c-8317-f2d4a96efbb8},1
- Registry path: HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\MMDevices\Audio\Render\{<identifier-removed>}\FxProperties\{45da5c30-2837-4ac4-b1e2-50acc3865974}\User

## Values and backup

The candidate API payload was 62,791 bytes, SHA-256 <private-artifact-digest>. Its 62,799-byte registry representation had SHA-256 <private-artifact-digest>.

The restored API baseline was 14 bytes, SHA-256 <private-artifact-digest>. The restored 22-byte registry value had SHA-256 <private-artifact-digest>.

Fresh backup was complete at 2026-10-13T13:27:59.6392372Z: private/native_property_test_20261013T132759546Z/backup_manifest.json. It records the exact property bytes, six context values, settings files and transaction logs; Access process count 0; active/default spatial GUIDs zero; and no spatial enablement. Operation result is property_round_trip_20261013.json. Prior review is dot_property_test_review_20261013.json.

## Boundaries and interpretation

No audio stream, playback, capture, spatial command, app launch, UI change, or other registry edit occurred. Post-restore checks verified the baseline, all six context values, app files, defaults and OFF state. This result establishes writable property persistence and immediate restoration only. It does not establish native PHRTF loading, personalized processing, Access integration, or restart persistence.

The exact approved property-only stage is complete. Approval for any other registry key/value remains separate; full registry import is not authorized. The 13:22 dependency_rollback_assessment_1322UTC.md was written while prior coordinator review was still pending; its wording is historical and superseded by the later review and result.

Static investigation remains active. REA DSP dependency evidence is exported and the session is closed: private/rea_dsp_dependencies_20261013.json and private/dsp_dependencies_20261013.rea-snapshot.json. The remaining static digest/cache-schema question is separate from this successful property round trip.

