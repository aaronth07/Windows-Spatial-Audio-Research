> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Windows Sonic and HRTF research inventory

Collected 2026-10-15 from existing `SpatialRendererProfileImport` reports/evidence and targeted top-level System32/SysWOW64 listings. File hashes and versions below identify the installed files on this machine; names and presence alone do not establish load/use.

## Existing research artifacts

| Relative path | Contents / limits recorded there |
|---|---|
| `SpatialRendererProfileImport/reports/2026-10-14_sonic_renderer_bridge.md` | Static audio-engine branch trace; points to `windows_sonic/audioeng_REA_evidence.json` and snapshot. |
| `SpatialRendererProfileImport/reports/2026-10-14_sonic_dataset_and_cache.md` | Default dataset inventory and prior parser/cache analysis; records unresolved response/ear semantics and update/restoration limits. |
| `SpatialRendererProfileImport/reports/2026-10-14_sonic_explicit_hrir_layout.md` | Explicit-HRIR serialization/validation notes; records missing physical-ear assignment and end-to-end import/update verification. |
| `SpatialRendererProfileImport/reports/2026-10-14_sonic_embedded_spatializer.md`; `reports/sonic_embedded_spatializer_review.md` | Embedded spatializer analysis and review; retain prior static evidence and limits. |
| `SpatialRendererProfileImport/windows_sonic/default_dataset_inspection.json` | `DefaultHrtfs.bin`: 4,227,116 bytes; SHA-256 `<private-artifact-digest>`; recorded 48 kHz, 2,048 positions, and two 256-sample serialized response arrays. Coordinate convention, ear order, response semantics, and native acceptance are marked unverified. |
| `SpatialRendererProfileImport/windows_sonic/hrtf_registry.json` | Saved read-only observation lists `Hrtf` pointing to `%SystemRoot%\System32\DefaultHrtfs.bin`; this inventory did not reread or change registry state. |
| `SpatialRendererProfileImport/windows_sonic/audioeng_REA_evidence.json`, `HrtfApo_REA_evidence.json`, `HrtfDspCpu_REA_evidence.json`, `SSDM_REA_evidence.json`, `embedded_spatializer_REA_evidence.json` | Prior static REA evidence bundles for the named system components. Matching snapshots are adjacent. |
| `SpatialRendererProfileImport/windows_sonic/sonic_cache_artifact_manifest.json` | Prior analysis-file manifest; states private inert Ghidra scope and that no runtime update experiment was performed. |

Prior reports list unresolved work around full HRIR metadata/consumer mapping and a supported cache refresh/restoration path. No new architecture conclusion is made here.

## Windows component files

Version is Windows FileVersionInfo where present. Hashes are SHA-256. System32 and SysWOW64 copies are separately identified because their sizes/hashes differ.

| Exact path | Bytes | File version | SHA-256 |
|---|---:|---|---|
| `<local-path>` | 4,227,116 | — | `<private-artifact-digest>` |
| `<local-path>` | 159,744 | 10.0.26100.4202 | `ffc12d088cc02441061cc2bb7f3b367fe8089662e0fd61ab98209ef6d43b6489` |
| `<local-path>` | 2,206,576 | 2024.0.42 | `4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9` |
| `<local-path>` | 176,128 | 10.0.26100.4768 | `<private-artifact-digest>` |
| `<local-path>` | 135,168 | 10.0.26100.4202 | `<private-artifact-digest>` |
| `<local-path>` | 77,824 | 10.0.26100.1 | `<private-artifact-digest>` |
| `<local-path>` | 90,112 | 10.0.26100.1 | `<private-artifact-digest>` |
| `<local-path>` | 2,013,720 | 10.0.26100.5074 | `<private-artifact-digest>` |
| `<local-path>` | 2,068,480 | 10.0.26100.5074 | `<private-artifact-digest>` |
| `<local-path>` | 637,792 | 10.0.26100.1 | `<private-artifact-digest>` |
| `<local-path>` | 679,744 | 2019.0150.2000.05 | `<private-artifact-digest>` |
| `<local-path>` | 110,080 | 10.0.26100.4202 | `<private-artifact-digest>` |
| `<local-path>` | 2,381,672 | 2024.0.42 | `<private-artifact-digest>` |
| `<local-path>` | 142,848 | 10.0.26100.4061 | `<private-artifact-digest>` |
| `<local-path>` | 536,376 | 2019.0150.2000.05 | `<private-artifact-digest>` |

`SqlServerSpatial150.dll` is a literal filename match for “spatial”; its file metadata identifies SQL Server 2019. No System32 or SysWOW64 top-level filename containing `Sonic` was found. `AudioEng.dll`, `audiosrv.dll`, `VirtualSurroundApo.dll`, `dfdts.dll`, and `dtsh.dll` are included because the existing reports/inventory reference them.

## Debugger availability

No `windbg.exe`, `windbgx.exe`, `cdb.exe`, or `ntsd.exe` was found on PATH. The conventional Windows Kits debugger directories `<local-path>` and `<local-path>` are absent; no registered WinDbg app package or matching `Microsoft.WinDbg*` package directory was found. This is a local tool-availability blocker for WinDbg-based follow-up.

Later in this research pass, the blocker was resolved by retaining the official WinDbg bundle and extracting its amd64 debugger tools under `../tools/windbg/amd64`. CDB version 10.0.29661.1004 was used successfully against isolated research probes. No debugger app installation was required. See the final research report for the runtime evidence.

Inspection was limited to named report/evidence files, top-level System32/SysWOW64 filenames and metadata, and conventional debugger install locations. No app/browser/UI was opened, no audio was used, and no configuration was changed.
