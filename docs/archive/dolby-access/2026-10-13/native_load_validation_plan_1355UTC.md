> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Native-object PHRTF validation proposal — 2026-10-13 13:55 UTC

The property-only test passed and the original property is restored. This next sequence is implemented and tested offline; it has not enabled Atmos, opened a stream, played or captured sound. prior coordinator's release is required before its runtime command. The desktop remains available to EQ APO's separately authorized screenshot task.

## Exact source and test

`scripts/native_object_validation.cpp` is a small C++ executable using IMMDevice -> ISpatialAudioClient -> ISpatialAudioObjectRenderStream, with one dynamic ISpatialAudioObject. It uses the consumer spatial format selected on the existing selected virtual endpoint endpoint. It uses neither a pro renderer nor the separate Microsoft ISpatialAudioObjectRenderStreamForHrtf interface. No suitable native WSSAPI stream implementation was found in the searched project source. The existing 11-channel ADM fixture needs a verified consumer decoder and is not used here.

Object source is mono IEEE float32 at 48 kHz, 480,000 frames (10 seconds), with exactly five samples at amplitude 0.01, approximately −40 dBFS. Nonzero frame indices are 24,000, 120,000, 216,000, 312,000 and 408,000. The rest are zero. Each source position occupies a two-second interval and remains stationary around its impulse; changes occur during silence. A processing quantum over 12,000 frames aborts so position changes settle at least 0.25 seconds before the next impulse.

| Order | Position | Windows XYZ, meters |
| --- | --- | --- |
| 1 | Front | 0, 0, −1 |
| 2 | Left | −1, 0, 0 |
| 3 | Right | 1, 0, 0 |
| 4 | Rear | 0, 0, 1 |
| 5 | Above | 0, 1, 0 |

Spatial stream category is SoundEffects, static mask None, minimum and maximum dynamic objects both 1. Exact object format must return S_OK and at least one dynamic object must be available; there is no automatic stereo/static fallback. Object volume is 1, with stimulus peak limiting the level. The program logs the actual accepted format/stream/object HRESULTs and metadata positions. Source generation is deterministic across processing block sizes.

Playback is `{<identifier-removed>}`. Capture is only the paired recording endpoint `{<identifier-removed>}` through shared WASAPI. Capture uses its actual GetMixFormat without requesting a device-format change, requires stereo 48 kHz, and stores the original negotiated format in WAV. It does not use a microphone, physical output or loopback fallback. The 13:27/13:28 inventories show playback persisted PCM16 and recording PCM24, unchanged across the transaction; the older PCM24 playback observation is historical. Capture's actual mix format remains to be observed.

Each run sends 10 seconds of data and records an additional 0.5 seconds of silent tail. Six runs are baseline1, baseline2, candidate1, candidate2, restored1, restored2: about 63 seconds of capture with only 30 impulse samples total. The render deadline is 16 seconds per run; a missing render event for 1 second aborts. The outer wrapper limits each owned native process to 20 seconds and each BAT process to 15 seconds; a hung process and its own children are terminated so finally restoration can proceed. Existing user apps are not terminated. Capture extent is capped at 10 MB per run. A later discontinuity/timestamp error aborts and releases the packet. Only owned streams are stopped/reset on exit. No default, routing, endpoint-format, service, driver, permission or Access UI change is included.

## Runtime sequence and rollback

`scripts/run_native_load_validation.ps1` defaults to local validation only. Runtime command, after prior coordinator releases the five exact source/executable hashes in `native_load_runtime_review_20261013.json`:

Historical workspace command omitted; use the public [example guide](../../../../examples/README.md).

1. Re-read defaults through MMDevice getters and explicitly require console AND multimedia IDs to equal `{<identifier-removed>}` before other preflight getters. Then verify endpoint formats, spatial subtype and both effect properties. Require Access closed, Atmos OFF, exact baseline14 API bytes, and DAFM VT_EMPTY. Make fresh independent property/context/app-file backups. Source and candidate hashes remain exactly those approved for the successful property test. No other property is changed.
2. Verify the existing BAT and SoundVolumeView hashes, then invoke `Enable spatial sound (Dolby Atmos for Headphones).bat` from `<local-path>`. Require active/default subtype `{<identifier-removed>}` by getters, same endpoint/formats, unchanged baseline/context/app files and DAFM VT_EMPTY. BAT output alone is insufficient. Run the helper's `--capabilities-approved` mode, which activates only the spatial client and queries formats/object capacity, without creating a stream.
3. Capture baseline1 and baseline2 on the named endpoints. Re-read baseline/context/app files and spatial state. Set/commit only the same approved 62,791-byte candidate through the already tested property-store API; require exact API/registry readback and unchanged other User values.
4. While that property is present, capture candidate1 and candidate2. The C# transaction runner invokes the bounded capture callback inside try/finally. Any callback failure still attempts the exact fresh property restore and Commit. Require API14/REG22 baseline hashes and all six User values after restore before any further capture.
5. Capture restored1 and restored2 with Dolby still enabled, then invoke `Disable spatial sound.bat` in the outer finally. Immediately before BOTH enable and disable BAT launch, freshly read MMDevice console/multimedia defaults and enforce the exact approved ID. If either differs or cannot be read, do not launch that BAT against a new device. Record the phase, exact observed IDs and `cleanup_incomplete=true` for blocked OFF cleanup. The unchanged C# inner finally attempts the fixed-property restore before its default checks; outer fixed-context/app backup verification runs independently of the spatial/default check. Require zero active/default subtype, baseline property/context/app files, unchanged defaults and formats before reporting complete cleanup. Any failure ends further testing and reports the actual HRESULT/state to prior coordinator; no alternate writer, elevation, hive import, full registry import or format reset is attempted.

Actual OS read-only query: Windows 11 Pro, version 10.0.26200. SoundVolumeView is the existing 2.36 executable, SHA-256 `<private-artifact-digest>`; the official change history documents a later 2.48 fix for Windows 11 25H2. Its compatibility here is untested. Failed enable/readback aborts before stream creation and attempts the selected OFF rollback; it is not permission to update or replace the tool.

Enable BAT SHA-256: `<private-artifact-digest>`.
Disable BAT SHA-256: `<private-artifact-digest>`.

## Discriminators and remaining limits

The comparison script accepts only six hash-matching stereo 48-kHz captures from a successful restored transaction. It rejects silence, nonfinite data and clipping, uses one shared stereo latency shift per capture, preserves gain and each ear's timing, and evaluates five fixed response windows. It reports relative waveform error, candidate-repeat error, baseline/restored variation and left/right energy difference. Both candidate repeats must exceed five times the maximum baseline/restored variation (with a 1e−6 numerical floor), while candidate repeats agree within that threshold. Fivefold separation is an operational discriminator, not a probability. A restoration mismatch prevents a positive comparison.

Direct decode/load evidence would require the candidate's native v2 conversion followed by successful Rosella deserialization and nonnull room/PCA pointers, without default-table fallback. Consumer DecodeMessageTuning `0x18011c450` logs v2 conversion, `Failed to decode raw tuning.`, `Deserialization of headphone coefficients failed`, and `Unable to obtain room and/or pca coefficients`. A v2 conversion message alone occurs before full coefficient loading and cannot prove it. An ISpatialAudioStream HRESULT, property-update log or lack of a visible error also cannot prove it.

Read-only Event Log provider inventory found no registered provider matching `*Dolby*`; this does not rule out private ETW/debug diagnostics. No tracing configuration, injection or binary patch is proposed. If existing non-mutating diagnostics expose the full load result, record it. Otherwise reproducible candidate-specific changes with restored controls establish changed native processing and support use of the only changed tuning field, while direct decode/load remains an inference and every-coefficient use remains unproven. No-change output does not prove rejection: branch, game/performance behavior, capture tap and default fallback remain possible explanations to diagnose.

Access UI/cache import and restart persistence are separate. `cache_schema_links_20261013.json` records the owned root/member schema and exact remaining link: compiled contract mapping/order plus typed date/Guid formatting and the complete activation/filename/digest config schema. The source capture UUID is a valid Guid; decoded hive InstallationId is not a directly parseable Guid and is not substituted into the filename. No hive or PHRTF cache edit is justified by the current evidence.

Offline checks completed: MSVC C++17 build with /W4 /WX; deterministic stimulus and end-of-stream zero fill across six block sizes; independent libsndfile decoding of float32 mono48000 with five exact impulses; no-overwrite and invalid-mode rejection; C# compilation/native layout/input guards; PowerShell parse/default local validation; synthetic common-latency tests in both directions, repeatable change, failed-restoration control and silence rejection. Actual Windows format/object/stream/capture behavior, switcher compatibility and callback restoration under a real stream failure remain pending runtime approval.

Sources: Microsoft spatial-object examples (historical reference unavailable)
