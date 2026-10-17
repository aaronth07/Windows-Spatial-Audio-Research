> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Independent review of the multistream lifetime chain

Reviewed only the new v2 lifetime-ID carrier in `../eqapo16/vbap_probe.cpp` and multistream conversion/reference work in `../eqapo16/tap_to_eqapo.py`, with the called geometry/reference helpers and current canonical capture/results. Root implementation files were not edited. No native endpoint, registration, live process, playback or recording operation was performed.

The canonical offline chain independently passed again: four source slots, six explicit object lifetimes, two captured native streams, 100 source updates and 48,000 producer-aligned frames. The actual Equalizer APO output matched the complete independent chain reference with maximum absolute error 6.322725383620803e-12 and relative error 3.8228573762672985e-7. All 4096 response taps and their final tails were included; unused output channels were zero. The master configuration hashes matched before and after. See `root_chain_review_run/result.json`.

## Confirmed finding

The reviewed converter snapshot checks `offset + frames <= len(raw)` but does not check `offset >= 0`. Python accepts a negative slice start. In a copied capture/reference pair, changing the first sample_offset from zero to -960 passed the converter's actual validation and sample-construction prefix and selected `raw[-960:-480]` as a valid 480-frame source. No PCM file content changed. This is a malformed-input validation bug; it did not affect the canonical source data or its numerical result. The test intentionally changed both JSON records to exercise the validator rather than the separate reference-equality check.

The reproduction in `root_chain_review_checks.py` runs the actual converter prefix through construction of the PCM/metadata arrays, stopping before the carrier is written or any worker starts. It does not claim that a complete malformed-input APO run was executed. The source snapshot hash is retained in the result.

Fixed and independently rechecked. The corrected converter requires positive integer frame counts; sample_offset exactly equal to the cumulative prior record extent; final cumulative extent equal to the PCM length; contiguous update ordinals; and a prior EOS requiring a new ID/generation on the next record for that slot. The same matched negative-offset fixture now fails in the actual converter prefix. The decisive large pan/gain lifetime-replacement case still passes unchanged. See `root_chain_review_fixed/result.json`.

The corrected converter also independently rejected all nine new corruption cases before native carrier creation: negative/overlapping/fractional offsets, EOS without replacement, generation gap, update gap, nonfinite volume, different PCM and truncated PCM. See `root_chain_fixed_negatives/result.json`.

## Implementation checks

- Geometry is consistent. Windows right/up/back `(x,y,z)` becomes front/left/up `(-z,-x,y)`. The 16 generated speaker directions surround the origin, and native VBAP uses inverse triplets in the correct column convention. Coefficients are clamped only within the stated boundary tolerance and normalized to unit squared gain. The supplied source volume is applied once after panning-weight interpolation. Distance is normalized away; no implicit distance attenuation is claimed.
- Lifetime handling is consistent for the declared fixed slot table. The v2 carrier stores explicit uint64 IDs independently of position/PCM pointers. IDs must be nonzero and unique within a block. A changed slot ID resets its previous pan weights and source gain before rendering. The converter validates stable `(stream, source_index, generation) → ID` ownership and rejects an ID assigned to another lifetime.
- An additional native v2 test makes replacement decisive: slot 0 changes to a distant speaker direction and volume .8→.1 with a new ID, while another slot continues. The first replacement sample matches the new pan/gain directly, and the full independent reference maximum error is 3.36845494205984e-10. The next same-ID packet still ramps normally. See `root_chain_review_checks/result.json`.
- Prior speaker-bus convolution tails remain intact across an object ID change. The APO convolvers retain the earlier bus PCM under fixed speaker responses; resetting a source's panner state does not reset those bus convolvers. This is the correct state separation for this architecture.
- Metadata boundary placement is consistent with the stated offline alignment. Updates are joined by update ordinal and slot, each update must contain all slots with equal frame counts, and each update extent is divided by their common GCD. A new update's metadata therefore starts at that update's first carrier sample. The ramp policy spans one carrier quantum; with varying transaction lengths that can be shorter than a producer transaction. It is not a claim of a producer-wide ramp in that case.
- Reference independence is useful. Native C++ computes triplet inverses by cross products; Python solves the speaker matrix with NumPy. The full-chain expected output convolves the independent Python panner result in float64 rather than the native bus, so the comparison covers both native panning and the actual APO processing. It still shares the selected SOFA responses and speaker geometry intentionally.
- Native carrier extents are checked before allocation/processing, including ID bytes in v2, truncation, trailing bytes, finite input/metadata, geometry indices, gain ranges, and finite output. Output/metric paths must be separate new paths. Runtime/write failures return nonzero. The probe writes PCM before metrics; a late metrics-file write failure may leave the already written PCM file, so it is not a transactional output writer. No such failure occurred in this review.

## Scope and timing limits

The actual multistream capture here uses 480-frame transactions. The separate packet cases exercise 1, 48, 127, 480, 1024 and 2048 frames in different runs; they are not a demonstrated mixed-length live capture. The panner common quantum is 480 for this canonical capture. Equalizer APO processing remains fixed at 128 through its own adapter, independently of the panner transaction length.

The two native streams are deliberately aligned at producer sample zero for this offline comparison. Their hardware clocks, device queues and live scheduling are not aligned or measured by this result. The timing metrics describe owned native worker processing, not full device latency. This path does not prove arbitrary game injection, generic stream synchronization, physical speaker localization or equivalence to direct per-object SOFA rendering.

Reviewed snapshots: vbap_probe.cpp SHA-256 `<private-artifact-digest>`; initial tap_to_eqapo.py `<private-artifact-digest>`; corrected tap_to_eqapo.py `<private-artifact-digest>`; native panner executable `<private-artifact-digest>`. No unresolved numerical or lifetime-state bug was found in the reviewed canonical chain.
