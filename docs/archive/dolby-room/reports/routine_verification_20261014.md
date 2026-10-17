> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Routine verification — 2026-10-14

All Python runs used <local-path>

## Existing tests

The first discovery attempt used SpatialRendererProfileImport/tools and found zero tests there. That was the wrong directory and is not counted as test coverage. The actual suite is in the absolute path <local-path>

After reading the selected test sources, ran offline modules individually with:
python.exe -m unittest discover -s SpatialRendererProfileImport\tests -p <test_file> -v

- SpatialRendererProfileImport/tests: 21 passed — test_dts_spac.py (6), test_sonic_dataset.py (4), test_registry_comparison.py (1), test_convert_sofa_dts.py (7), test_profile_capture_inputs.py (1), test_native_capture_analysis.py (2).
- DolbyHeadphoneRoomResearch: 18 passed.
- DolbyCoefficientResearch: 12 passed.
- DolbyHeadphoneRoomResearch/tools/test_dolby_room_control.ps1: returned 8; all eight in-memory transaction cases completed.

Excluded runtime-sequence tests: test_profile_capture_getters.py, test_native_dts_validation.py, test_dts_control_cli.py, and SpatialRendererProfileImport/tests/test_dts_transaction.ps1.

## CLI refusal checks

- make_directional_gain_probe.py with an existing project-local output directory: rejected with “Use a new project-local candidate directory” before reading the baseline payload.
- make_directional_gain_probe.py with a non-existent output path outside its project root: rejected with the same message; no directory was created.
- inspect_dolby_native_decode.py with an existing output file: exited 2 with “Output exists”; the existing file hash was unchanged. Its output-exists guard precedes inspect().
- inspect_dolby_native_decode.py with make_directional_gain_probe.py as an unapproved input and a new report output path: rejected with “Unreviewed API fixture”; no output file was created. The input size/hash check occurs before the native DLL is opened.

The retained candidate API payload SHA-256 remains <private-artifact-digest>. No candidate-generation success path or recording path was invoked. No native DLL was loaded, and no audio capture, setter, or provider operation was run.
