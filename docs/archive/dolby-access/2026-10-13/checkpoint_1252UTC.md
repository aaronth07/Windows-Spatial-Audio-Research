> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Dolby Access PHRTF checkpoint — 2026-10-13 12:52 UTC

The endpoint-property test is approved. Opening the user effect store with STGM_READWRITE returned S_OK at 12:39:42 UTC, but no SetValue or Commit was attempted. Opening a writable interface is not evidence that a write will succeed.

Fresh target registry/API backups, the six-value User-context snapshot/export, and LocalCache/settings.json were verified at 12:45 UTC. Settings/settings.dat and LOG1/LOG2 could not be read because they are locked; a normal FileStream read sharing ReadWrite/Delete also failed. The fresh manifest explicitly records backup_complete=false. The live write is stopped pending coordinator review or a complete consistent backup. The running Access process was not closed or killed. No live property, registry, app state, routing, format, or audio stream changed; no rollback was needed.

Static work continues. The matching Microsoft framework PDB now names String.op_Inequality for the cache digest comparison and String.IsNullOrEmpty for the filename guard. MD5 uses ASCII bytes and uppercase X2 formatting; exact serialized JSON member order and accepted cache state are unverified. The profile identifier helper previously labeled as parsing is a nullable value constructor/getter, not textual parsing. Its exact type association is being checked before renaming it.

Current evidence root: <local-path>

- Report
- Approved test plan
- Fresh backup result
- Write-access probe
- Candidate verification
- Digest encoding
- PDB symbols

A prior reviewer (local chat <identifier-removed>) is the requested communication channel. The earlier durable Luna relay failed with an app AbsolutePathBuf error. The question is whether the full UWP app-hive backup belongs to this temporary endpoint-only test or to the later app-file/UI/persistence test; the boundary has not been relaxed. Audio comparison and native acceptance remain planned work.
