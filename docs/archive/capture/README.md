> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Windows Spatial Sound to SOFA

A standalone Windows x64 application that measures binaural responses from native Windows Spatial Sound objects and exports SOFA. Download and open `WindowsSpatialSound2SOFA.exe`; no Python, compiler, virtual audio driver or additional runtime installation is required. The Windows API minimum is Windows 10 version 1809; Windows 11 x64 is also targeted. See `VALIDATION.md` for runtime coverage.

Select a playback device with Spatial Sound enabled in Windows. The app accepts the active provider, including Windows Sonic for Headphones, Dolby Atmos for Headphones and DTS Headphone:X. Check audio verifies the selected device and provider. Sound settings opens Windows settings; the application never changes providers, device formats, defaults, profiles, EQ or routing. Provider availability and licensing remain managed by Windows and the provider.

Playback loopback records the selected playback endpoint using WASAPI. An optional paired recording endpoint supports virtual audio routing. No microphone is selected automatically. Run pilot compares repeated positions at the selected peak level and twice that amplitude (6.02 dB higher); Measure sphere acquires the selected fixed grid. Pause finishes the current position; Resume rechecks settings. Stop retains completed positions and raw data. Export SOFA processes a captured session without requiring Spatial Sound to remain enabled.

The default near-uniform grid contains 710 unique directions: an icosphere, cardinal positions and a 5Â° horizontal circle. A 10Â° latitude/longitude grid contains 614 directions. CSV imports use `azimuth,elevation,radius` in degrees,degrees,metres, with an optional header. SOFA 0Â° azimuth is front, 90Â° left,+90Â° elevation above. Native source coordinates are right/up/back in metres.

Capture uses one native dynamic object, 48 kHz mono float excitation and stereo float WASAPI recording. Endpoint settings fingerprints, spatial provider, default-device changes, negotiated format, clipping, discontinuities and timestamps are checked. Other audio mixed into the endpoint is also recorded. The app does not modify the selected renderer or remove its room processing.

Processing uses a matched exponential sweep inverse. Relative ear timing and level are preserved. SimpleFreeFieldHRIR exports 4096 samples with a shared onset trim, no onset fade and a 64-sample cosine end fade. GeneralFIR retains the full deconvolved interval. Receiver geometry is nominal. The virtual free-field convention does not prove anechoic equivalence. SOFA output contains generic application metadata, not account names, device IDs, profile IDs, machine paths or imported subject descriptions.

Preferences, sessions, logs, cache and license notices are stored under `%LOCALAPPDATA%\WindowsSpatialSound2SOFA`. The default session location is its `sessions` directory; another output folder can be selected. One-file runtime extraction uses Windows per-user temporary storage and is cleaned up on normal exit. No preferences or captures are written beside the download. Device IDs are stored only in local preferences/session records for device selection and reproducibility; they are not shipped with the application or exported in SOFA metadata.

Invalid preference fields fall back to defaults. When malformed preferences are replaced, the original file is retained beside them as `preferences.invalid.<identifier>.json`.

The Win32 interface and audio/processing worker are separate processes. The one-file download bundles the native capture engine, worker, Python runtime and dependencies. Source, third-party licenses and build instructions are included in this repository. No personal profiles, recordings, generated SOFAs or local device configuration belong in Git.

## Build and test

Building from source requires Windows x64, Visual Studio C++ tools with a Windows SDK, and Python 3.8â€“3.11. These are developer requirements only.

Historical workspace command omitted; use the public [example guide](../../../examples/README.md).

The standalone output is `dist/WindowsSpatialSound2SOFA.exe`. For development after native compilation, run `.\.venv\Scripts\python.exe src/app.py`. `build.ps1` discovers the installed compiler/SDK; no account or machine paths are fixed in source. Third-party license texts are preserved under `third_party` and embedded in the standalone application. No license for the application's own source has been selected.

Technical references: Windows spatial objects
$process = Start-Process -FilePath .\WindowsSpatialSound2SOFA.exe -ArgumentList $arguments -WindowStyle Hidden -Wait -PassThru
$process.ExitCode
```

Place `--log` before the worker action. Available actions are `devices`, `inspect`, `capture`, `export`, and `verify-resume`; append `--help` to an action for its arguments. Without `--log`, output goes to `headless.log` in the application data directory. Each invocation replaces its log, so use separate files for concurrent actions.

For automated captures, pass the IDs returned by `devices` using `--playback` and optionally `--recording`, and choose `--mode loopback` or `--mode paired`. Export uses `export --session <session-folder> --convention SimpleFreeFieldHRIR` or `GeneralFIR` and does not inspect audio devices. Session `control.txt` accepts `pause`, `run`, or `stop`; run `verify-resume --session <session-folder>` before resuming a paused capture. Prefer atomic file replacement when updating the control file.
