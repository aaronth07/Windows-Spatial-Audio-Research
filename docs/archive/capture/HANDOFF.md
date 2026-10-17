> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Continuation handoff

This repository contains the generic standalone Windows Spatial Sound to SOFA capture application. Continue application development in this repository. The separate personalized-headphone reverse engineering project is outside this archive.

## Requirements and decisions

Use ISpatialAudioClient / ISpatialAudioObjectRenderStream with a dynamic object positioned in three dimensions; capture stereo through WASAPI loopback or an explicitly selected recording endpoint. Do not replace this with channel-based rendering or Dolby Atmos Renderer. Accept any active Windows Spatial Sound provider and stop with a useful error when it is disabled. Never change provider, defaults, formats, profiles, EQ or routing automatically.

Keep UI and recording/processing separate. The Win32 UI launches a packaged worker; the worker launches the native capture engine. A single downloadable EXE contains all runtime dependencies. Permanent preferences, licenses, cache and sessions default to LocalAppData/WindowsSpatialSound2SOFA. No machine configuration is shipped.

SimpleFreeFieldHRIR defaults to 4096 samples. The user accepted this length in Virtuoso; a 14080-sample export was rejected. Preserve relative timing and gain, trim with one common onset, use no onset fade and only a 64-sample end fade. Do not normalize each direction or ear separately. GeneralFIR retains the full interval as an archival option. Neither the convention nor this virtual measurement establishes acoustic anechoic equivalence; acoustic verification was explicitly deferred.

Default grids contain 710 near-uniform directions or 614 latitude/longitude directions; CSV import is supported. Version 1.1.0 validation includes six cardinal directions in both recording modes and a complete 710-direction loopback capture; see VALIDATION.md for parameters and limits. Personal full-sphere measurements and the accepted earlier SOFA are deliberately excluded.

## Source map

- src/app.py: one-file bootstrap and AppData/runtime paths.
- src/desktop_app.cpp: native interface, device selectors, worker pipes, progress, pause/resume/stop.
- src/worker_entry.py and src/desktop_worker.py: packaged worker and JSON event protocol.
- src/spatial_capture.cpp: read-only provider/settings inspection, dynamic object rendering and WASAPI recording.
- src/configuration.py: per-user preferences.
- src/measurement.py: grid, stimulus, session capture and SOFA processing.
- src/export_hrir.py: conversion of existing SOFA responses with generic metadata.
- src/sofa_io.py: SOFA access through relative ASCII filenames for Windows Unicode-path compatibility.
- tests/: deterministic processing/configuration/worker tests.

Build and run commands are in README.md. Read VALIDATION.md for tested scope and limits. SHA256SUMS.txt covers this archive's files. The executable is included at archive root; repository sources are under repository/. No GitHub repository was created or published. The local source directory was initialized as an independent Git repository. Git history is excluded from the transport archive to avoid account metadata; run git init -b main after extraction if needed. No application source license has been chosen.

## Fixes made during packaging

Removed account paths, fixed devices, personalized profile hashes and Dolby-only checks. Added endpoint enumeration, generic active-provider detection, per-user preferences and bundled runtime. Settings fingerprints detect changes without embedding an expected personal profile. Added compatibility with older session fingerprint field names while writing generic names in new sessions. Removed subject descriptions, personal paths and original imported history from SOFA metadata. Fixed command-line arguments being overwritten by asynchronous preference loading and numeric preferences failing to match UI selection labels.

## Next work

A new chat can maintain this app independently. Preserve existing timing/export behavior. Expand cross-provider and clean Windows testing when available; host tests do not establish compatibility with every driver/provider. Do not bundle personal captures, endpoint configuration, profile material or private investigation logs. Do not publish without a fresh privacy and licensing review and the user's publication instruction.

## Version 1.1.0 continuation

See CHANGELOG.md for fixes. The standalone executable now also supports `--headless --log <file> <worker-action>` for command-line testing and operation. The capture thread uses Windows multimedia scheduling. SOFA file access handles Unicode directories and imported filenames through relative ASCII names, with working-directory restoration after library errors. Invalid preferences fall back to usable defaults, and malformed originals are retained when replaced. SOFA write and read-back failures leave the final output name available for retry.

The original long-run discontinuity, its retained raw data and subsequent test results are documented in VALIDATION.md. No private capture, configuration or diagnostic log belongs in the distributable source archive.
