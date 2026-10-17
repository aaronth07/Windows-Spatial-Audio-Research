> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Native DTS room preset comparison

Use only the existing selected virtual endpoint render/paired capture endpoints. Do not change the global default, spatial provider, device profile, Dolby payload, driver, service, vendor application, or physical device.

Take fresh verified backups of the current DTS runtime, device profile and transaction counter before any write. Require the exact stock runtime and currently selected DTS provider. Validate the source fixtures and both candidate hashes first. Validate the new candidate with the pinned native parser. Run the copied transaction implementation against memory before using it on the endpoint.

Record stock; personalized FIRs with the saved SD1 room sends; the same FIRs with SD2 room sends; stock restored. At each state render the existing −34 dBFS one-second sweep as a dynamic spatial object at 30 degrees left, 90 degrees left, and rear, repeated twice, with the existing pre-roll and two-second tail. Only the first stereo auxiliary vector set differs between personalized candidates. All personalized FIR bytes, room header, panner and unresolved second auxiliary set must match.

Restore stock and its exact fresh transaction counter between candidate states and in a finally block. Stop on any backup, validation, write or stream failure. Compare fresh registry snapshots and global defaults after cleanup, then inspect the provider and stock hash again. Save capture and restoration evidence even after failure. The installed vendor app is read-only during this experiment.

SD1 candidate SHA256: `<private-artifact-digest>`.

SD2 candidate SHA256: `<private-artifact-digest>`.

The user's existing authorization permits changes on selected virtual endpoint. This plan adds a room-send vector set recovered offline from the cached SD2 asset; it does not extend the write scope.
