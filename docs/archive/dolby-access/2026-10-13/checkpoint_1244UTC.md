> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Consumer PHRTF checkpoint — 2026-10-13 12:44 UTC

This is a report-only checkpoint of the approved live-test preflight. No property value or Dolby Access setting was changed.

The approved read/write access probe at `2026-10-13T12:39:42.2708308Z` returned `S_OK` from `OpenUserPropertyStore(STGM_READWRITE)`, despite the process token not being an administrator. This proves only that the writable property-store interface opened. The probe used `read_effect_property.ps1` with its approval-checked `ProbeApprovedWriteAccess` switch; that code path opens and releases the interface and has no setter or commit call. Evidence is `effect_property_write_access_20261013.json`. The approved endpoint/context/property are recorded in `approved_property_test_20261013.json` and `native_import_test_plan.md`.

Fresh backup preparation created `<local-path>`. The partial directory contains the exact 22-byte registry value, 14-byte API payload, scoped context export, context-value JSON, and a copy of `LocalCache\settings.json`. The script then failed while copying `Settings\settings.dat`, which is locked by the running Dolby Access process (PID 10124; started 2026-10-13 06:26:59 local). A separate FileStream read attempt allowing `ReadWrite|Delete` sharing also failed with a sharing violation. The approved script did not produce `backup_manifest.json` or root `native_property_test_backup_20261013.json`; this directory is an incomplete backup and is not qualified for write/rollback.

The endpoint/context/property target is `Endpoint {<identifier-removed>}`, context `{45da5c30-2837-4ac4-b1e2-50acc3865974}`, property `{1b4dab55-b1fb-4d8c-8317-f2d4a96efbb8},1`. The writable interface probe made no `SetValue`, `Commit`, registry write, stream activation, playback, or capture call. No rollback is needed because no live value changed.

**Current decision:** stop the approved property write until a consistent verified backup is available or the coordinator reviews whether the locked settings file is a required dependency. Do not ask <identity-removed> while AFK, bypass the lock, close or kill Dolby Access, or broaden registry authorization. Future coordination is through dot or Luna as <identity-removed> requested. No app/settings state change occurred.

Exact paths:

- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
