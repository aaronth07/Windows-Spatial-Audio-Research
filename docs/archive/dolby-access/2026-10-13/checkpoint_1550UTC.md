> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Checkpoint — 2026-10-13 15:50 UTC

The native sequence result from 15:02:34–15:03:55 UTC remains unchanged: the reviewed tuning property produced reproducible capture differences, and the original property value, current registry values, app files, endpoint inventory and Atmos-off state were verified after restoration. The native evidence and hashes are preserved in `checkpoint_1510UTC.md`. No live operation was repeated for this report update.

Offline capture characterization provides more detail about that result. A scalar gain fit leaves 29.831% relative L2 residual against the candidate, and changes vary with direction and ear. Common latency was already handled with the shared alignment from the original comparison. The raw captures are float WAVs whose samples lie on a PCM16 grid. The 450-ms pre-impulse region is identical between profiles, with a one-LSB peak (1/32768) and RMS around 1.42–1.44e−5, so full-raw spectral fits are noise-sensitive. At three times quiet FFT RMS, a common-filter fit using available ears from at least two directions has 1,989 supported bins and 6.037% residual; this is a diagnostic model fit, not proof of the transfer mechanism.

The optional 28-ms rectangular early-response analysis retains 100.000%, 98.315%, 98.464%, 93.998% and 97.722% of the observed profile-difference energy for front, left, right, rear and above. It includes early reflections and is not an isolated direct HRTF. At the three-times quiet threshold, its common-filter fit across 787 bins supported in at least two directions leaves 19.925% residual. Right-versus-left integer correlation lag is zero; measured sub-sample changes stay below one sample. Phase-fit estimates are inconsistent/sparse and do not resolve ITD. These observations show direction-dependent signal change in the captured path but do not establish listener physiology, perceptual improvement, or direct internal coefficient decoding.

The corresponding characterization report is `native_capture_characterization_1540UTC.md`. Script: `characterize_native_captures.py`, SHA-256 `<private-artifact-digest>`. Machine result `characterization.json` is 27,512 bytes, SHA-256 `<private-artifact-digest>`. Supporting outputs remain at their original locations:

| Artifact | SHA-256 |
| --- | --- |
| `direction_summary.csv` | `<private-artifact-digest>` |
| `frequency_comparison.csv` | `<private-artifact-digest>` |
| `full_frequency_bins.npz` | `<private-artifact-digest>` |
| `spectral_comparison.png` | `<private-artifact-digest>` |
| `spectral_comparison.svg` | `<private-artifact-digest>` |
| `optional_early_response.json` | `<private-artifact-digest>` |
| `optional_early_frequency_comparison.csv` | `<private-artifact-digest>` |
| `optional_early_frequency_bins.npz` | `<private-artifact-digest>` |
| `optional_early_response.png` | `<private-artifact-digest>` |
| `optional_early_response.svg` | `<private-artifact-digest>` |

Static metadata now maps key PHRTF types and JSON fields: `Checksum` to `correlation_id`, capture dates to property-specific `DateTimeOffset`/`IsoDateTimeConverter`, and `Uuid` to `Guid`/`GuidConverter`. The activation enum converter writes a Boolean; Guid-to-string is linked to a PDB symbol. Metadata order does not establish JSON emission order. The global date converter trace must not be generalized to the attributed capture-date properties; effective converter precedence remains unresolved. See `access_native_metadata_contract_1525UTC.md` and `access_native_metadata_models_20261013.json` (SHA-256 `<private-artifact-digest>`).

Next static target is the activation and Guid converter path. `native_persistence_lifecycle_proposal_1525UTC.md` was submitted to prior coordinator; no new live execution is released. No cache acceptance, settings mutation or new audio capture is established by this offline work.
