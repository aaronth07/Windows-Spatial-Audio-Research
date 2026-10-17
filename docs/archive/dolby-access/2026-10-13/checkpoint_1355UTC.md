> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Checkpoint — 2026-10-13 13:55 UTC

## Property round-trip verification

The property-only candidate round trip remains verified. Candidate and restored readbacks match their saved hashes; the endpoint inventory was identical before and after, and Atmos remained off. The operation establishes property-store persistence and exact baseline restoration only. It does not establish native PHRTF loading, personalized processing, Dolby Access UI integration, restart persistence, or acoustic performance.

The current persisted playback format is stereo 48 kHz PCM16. The paired recording format is stereo 48 kHz PCM24. The two formats and endpoint inventory were identical before and after the transaction. The older playback PCM24 reading belongs to the earlier observation and is not current. Evidence: property_round_trip_verification_20261013.json.

## Offline native-object validation preparation

The native C++ object helper, C# property transaction wrapper, PowerShell runner, and capture comparison source were compiled and tested offline. The preparation record reports no streams opened and no spatial switcher batch files run. Native runtime has not been released. prior coordinator is reviewing the five pinned hashes and the six-capture baseline1/baseline2, candidate1/candidate2, restored1/restored2 sequence in native_load_validation_plan_1355UTC.md. The desktop remains available for the separate EQ APO task.

The plan is tied to the existing playback endpoint selected virtual endpoint and paired recording endpoint selected virtual endpoint. It requires actual readback before proceeding and does not change endpoint formats, defaults, routing, or Access UI. Operating system readback is Windows 11 Pro build 26200. The existing SoundVolumeView 2.36 compatibility is unverified; the runtime plan treats failed enable/readback as an abort and does not authorize updating the utility. No run or audio capture has started.

## Storage and diagnostic limits

The known source profile has root personalized_hrtf, with children phrtf_capture_metadata and virtualizer_parameters. Their source names/order and the known native field links are recorded in cache_schema_links_20261013.json. The typed Access JSON contract, output member order, date/Guid formatting, complete activation/config schema, and cache acceptance remain unresolved. The source capture UUID is a valid Guid. The decoded InstallationId is not directly parseable as a Guid and is not substituted.

The read-only registered Event Log provider query found no provider matching *Dolby*. This bounded inventory does not rule out private ETW or other diagnostics. No tracing configuration, injection, binary patch, stream, or live setting change was made in this checkpoint.

The 13:27 property round trip and exact immediate restore remain the only demonstrated live behavior. Native decode/load, object-stream behavior, candidate-specific processed output, Access cache import, restart persistence, and personalized playback remain pending the separately reviewed validation sequence. See native_load_preparation_20261013.json, property_round_trip_verification_20261013.json, existing_dolby_log_inventory_20261013.json, and the primary-authored native_load_validation_plan_1355UTC.md.
