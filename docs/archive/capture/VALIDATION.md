> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Validation

Version 1.1.0, tested on Windows x64 on 2026-10-16. Tests used command-line processes and session files; no computer control was used.

- Twenty-four deterministic tests passed, covering coordinates, both default grids, CSV input, matched sweep inversion, relative ear timing and gain, shared onset trimming, SOFA round trips, private metadata exclusion, timestamp validation, silent-ear rejection, failed-session rejection, export retries, preferences, worker parameters, pilot levels, Unicode file access and owned-process shutdown.
- Both native release builds passed `/W4 /WX` with the static C++ runtime. Python source compilation and dependency consistency checks passed.
- The standalone executable enumerated devices, inspected the selected endpoint and ran captures with a Windows-only PATH and no external Python executable. Fresh application data and malformed preferences were exercised in isolated directories. Third-party license notices were copied from the bundle into application data.
- Six native dynamic-object positions were captured through playback loopback and a paired recording endpoint. Each exported finite, nonzero `6 x 2 x 4096` SimpleFreeFieldHRIR data. GeneralFIR export was also exercised.
- Pause, settings verification, resume and stop were exercised through the session control protocol. Stop retained two completed positions and exported their responses.
- The final executable exported an existing recording without inspecting audio devices, rejected an existing output and returned an error for invalid capture arguments. The default endpoint, with Spatial Sound disabled, was rejected without changing its settings.
- An initial long capture stopped after 552 completed directions when WASAPI reported a discontinuity or timestamp/device-position error. Raw data were retained and automatic SOFA export was blocked. The capture thread now registers with the Windows multimedia scheduler; registration and high-priority selection succeeded during the repeated bounded tests. Gap flags and expected/actual device positions are now recorded on failure. The original failure's precise cause was not established.

- The repeated 710-direction near-uniform grid completed through the standalone executable using a 0.25-second sweep, 0.25-second response tail and -40 dBFS excitation. The export had shape `710 x 2 x 4096` and no rejected directions. Endpoint inspection matched before and after both the bounded tests and this full run. This passing repeat does not establish the cause of the first discontinuity.

- The final Unicode-compatible package re-exported the retained full-grid recording. Every sample and source coordinate matched the previous successful export exactly. The native capture engine was byte-identical across that packaging rebuild.
- The packaged two-level pilot completed 18 positions per level at -34 dBFS and twice that amplitude. Both SOFAs and the finite level/repeatability comparison report were produced.
- Packaged export succeeded with accented and Chinese characters in the application-data and output paths, and with a Unicode temporary directory for one-file extraction. Source tests also covered Unicode SOFA filenames and working-directory restoration after errors.
- The final privacy inspection checked distributable application source, tests, bundled files and 40,136 Python code objects. No tested personal account path or selected endpoint identifier was found, and no absolute code filenames were detected. Actual captures, preferences, identifiers and private validation logs remain outside the release archive. Required third-party copyright attributions are retained.

Limits: the tests use the existing host and its active provider and drivers. A clean Windows VM, other provider/driver combinations and interactive GUI behavior were not retested. The interface changes received code review and native compilation. Nominal receiver geometry, SOFA structural validity and virtual renderer capture do not establish acoustic anechoic equivalence or personal HRTF accuracy. The application source license remains undecided, and no public publication was performed.
