> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Checkpoint — 2026-10-13 16:17 UTC

The previously executed six-run native Spatial Sound validation remains restored and is the only live runtime sequence completed. Offline capture characterization and metadata findings are summarized in the current report. No live settings, endpoints, audio or cache state were changed in this reporting update.

The submitted source review describes a distinct four-run OFF/ON test: baseline, candidate before toggle, candidate after toggle, and restored. Both candidate captures use Dolby enabled; the intervening OFF checkpoint only reads the property. The candidate remains installed only during that proposed sequence; C# cleanup restores the original property before endpoint verification. The source-review report is present, but its separate release record is absent. The earlier six-run approval does not release this test. No runtime execution is authorized/released by the available source review, and the sequence has not run.

Static PHRTF metadata maps `Checksum` to `correlation_id`, dates to property-specific `DateTimeOffset`/`IsoDateTimeConverter` attributes, and `Uuid` to `Guid`/`GuidConverter`. Activation serialization is Boolean and the Guid-to-string routine is PDB-bound. Effective property-specific date conversion and final JSON output order remain unresolved. prior coordinator is reviewing the lifecycle sources; execution remains held.

The characterization report now uses ASCII `+/-` for its phase bounds. The updated ZIP contains only five members and each source hash matches:

| Member | SHA-256 |
| --- | --- |
| `native_capture_characterization_1540UTC.md` | `<private-artifact-digest>` |
| `direction_summary.csv` | `<private-artifact-digest>` |
| `frequency_comparison.csv` | `<private-artifact-digest>` |
| `spectral_comparison.png` | `<private-artifact-digest>` |
| `optional_early_response.png` | `<private-artifact-digest>` |

ZIP path: `reports/2026-10-13/native_capture_review_package_20261013.zip`, 1,242,086 bytes, SHA-256 `<private-artifact-digest>`.
