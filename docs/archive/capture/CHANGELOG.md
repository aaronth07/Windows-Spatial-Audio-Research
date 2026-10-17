> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# 1.1.0

Improves capture controls, preference handling and export reliability, and adds command-line operation to the standalone application.

- Added headless device inspection, capture, export and resume verification with JSON event logs and process exit codes.
- Corrected pilot excitation levels to use the selected peak and a second sweep at twice the amplitude.
- Serialized interface audio checks and prevented delayed preferences from changing controls during an export.
- Added preference field validation, malformed-file preservation and retries for transient file replacement locks.
- Fixed SOFA reads and exports in directories and filenames containing non-ASCII characters.
- Made SOFA writes temporary until round-trip validation completes, allowing retries after write or plotting failures.
- Rejects measurements with either ear silent or noise-dominated, and blocks export from failed sessions.
- Stops the owned capture process when progress handling fails; rejects invalid session labels, unknown grids and nonfinite timestamps.
- Registers the capture thread with the Windows multimedia scheduler and records packet flags and device positions when a capture gap is detected.
- Supports explicit playback selection when an unrelated default playback role is absent.
- Added missing importlib_resources, libdivide and ARPACK license notices, and expanded processing and packaging regression tests.

