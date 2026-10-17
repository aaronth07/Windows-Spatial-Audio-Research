> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Installed spatial audio component inventory

Collected 2026-10-14 using read-only package, file, and registry inspection. Output files are `inventory.json` and this report.

## Installed DTS package

`DTSInc.DTSSoundUnbound` version `2026.3.31.0` is registered as an x64 MSIX package at `<local-path>`. Its manifest SHA-256 is recorded in `inventory.json`. The separate `<local-path>` directory contains Neural UpMix documentation and its uninstaller, with no DTS Headphone:X rendering DLL observed in that directory.

## Observable component identities

The package manifest explicitly registers DTS Headphone:X and DTS:X Ultra audio encoders to `x64\DtsHrtfEnc.dll`, a DTS:X for home theater encoder to `x64\DtsxHdmiEnc.dll`, and a DTSX decoder to `x64\DTSXDecoder.dll`. The manifest separately exposes `HeadphoneEQConverter.Converter` from `HeadphoneEQConverter.dll` and `SpatialSoundPlayer.SpatialEditor` from `SpatialSoundPlayer.dll`.

Headphone-model EQ is indicated by the `HeadphoneEQConverter` class and `Data\HeadphoneTuning\db_hpeq_*` resources. The related encrypted resources and databases are listed and hashed without decrypting or parsing their contents. No inspected package manifest or resource filename explicitly identifies listener-personalization data. This is a limit of the inspected identities, not evidence that such a feature is absent.

The manifest declares `DTSXDecoder.DTSXDecoder` as an `audioDecoder` media codec. That declaration establishes a packaged decoder identity only; it does not establish registration or runtime use by the Windows audio engine.

## Windows audio and endpoint registrations

The targeted System32 listing contained `HrtfApo.dll`, `VirtualSurroundApo.dll`, `dfdts.dll`, `dtsh.dll`, and `SpatialAudioLicenseSrv.exe`; file versions and SHA-256 values are recorded as candidates. Their presence and names alone do not identify active renderer selection or processing behavior.

A read-only scan of `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\MMDevices\Audio\Render\{endpoint-guid}\FxProperties` found Windows audio effect and Equalizer APO entries. Linked class registrations included `WM GFX APO`, `WM LFX APO`, EqualizerAPO Pre-Mix and Post-Mix, and `CplPage Class`, with their in-process server paths listed in the JSON. No DTS- or Sonic-named endpoint provider value or DTS CLSID was observed in the inspected render endpoint properties and linked registrations.

## Reproducible inspection commands

Run in PowerShell:

Historical workspace command omitted; use the public [example guide](../../../../examples/README.md).

## Limits

The inventory did not launch DTS Sound Unbound, activate COM objects, play or capture audio, inspect private app databases, reverse engineer binaries, or change registry, endpoint, configuration, or provider state. Endpoint registry inspection was limited to audio render endpoint `FxProperties` and linked CLSID registrations. Package resource identity is not a runtime or architectural feasibility result.
