> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Dolby Access PHRTF Enabler

Load personalized headphone profiles into **Dolby Atmos for Headphones** or **DTS Headphone:X** on Windows, with native room controls and automatic backups.

Version **2.0** adds DTS personalization and room presets, plus Dolby 200% and 400% room/direct modes.

Dolby PHRTF controls

## Run

Download the portable EXE from the latest release, while retaining saved room taps and feedback. Overall makeup gain remains 0 dB. Apply requires confirmation. Repeated Apply and switching modes use the saved full-gain reference; On, Restore, and reset remove the attenuation. Load a full-gain PHRTF containing room output first. These modes were verified by the user; no new acoustic capture is bundled. Custom 150% scales room taps alone.

## DTS Headphone:X

1. Select the playback device and binaural SOFA file, then **Verify and convert**.
2. Enable DTS if offered. Choose **Original**, **Balanced**, **Spacious**, or **Off** under **Room preset**.
3. Click **Apply profile**, or turn **Custom DTS profile** On.

Room changes preserve personalized FIRs and the panner, and write the combined profile and transaction notification together. Original restores room sends saved before the first DTS change. Balanced/Spacious are extracted from the installed DTS Sound Unbound package; vendor preset binaries and decoding keys are not bundled. The verified mapping is for Sound Unbound 2026.3.31.0 and the compatible 23-node Windows headphone grid. If mapping is unavailable, an explicit override retains Original room sends instead of guessing a named preset.

Applying a SOFA disables the endpoint's DTS headphone-correction filters. The app warns when it detects these filters active. The removed EQ dropdown did not represent the vendor app's EQ profile selector. Off or **Restore** returns the saved runtime profile, device EQ, and counter. Changing a vendor EQ/profile can replace the injected configuration, so reapply afterward.

The converter accepts binaural SimpleFreeFieldHRIR and GeneralFIR with a full-sphere grid, one source distance, and integer sample delays. It resamples to 48 kHz using one set of interpolation weights for both ears. Conversion details report dimensions, sample rate, delay, target grid, and native format-validation result. Error details explain rejected inputs. Status reports the selected provider and custom profile state, rather than claiming to verify installation.

## Persistence, reset, and overrides

Settings remain on the selected device after exit. Audio reinitialization, device recreation, and vendor profile changes may require reapplying. The app does not run a background monitor.

Backups are stored in `%LOCALAPPDATA%\AtmosProfile\<device GUID>`. Each write retains a transaction snapshot, while the original restore point is kept separately.

- **Reset configuration** restores saved Dolby room fields, clears Dolby state and backups, and saves the current personalized profile as the new Off baseline. DTS is preserved.
- **Reset DTS configuration** restores the saved DTS runtime profile, EQ and counter, then clears DTS conversions and backups. Dolby is preserved.

Both resets require confirmation in the app. Unfamiliar versions, external configuration changes, and damaged backup records offer explicit overrides where structurally valid data can be preserved. Invalid backup records are retained locally. Unknown encoder versions skip private native calls under override; malformed coefficient framing, checksums, or unsafe room edits are still rejected.

## Recorded comparisons

These are matched native spatial-object recordings at 30 degrees left, 0 degrees elevation, 1 m. Frequency response uses mean left/right power with 1/6-octave smoothing; impulse-response energy uses 2 ms blocks. Axes match and no gain normalization is applied. The measurements demonstrate renderer behavior for the tested profiles, rather than personal fit or listening quality.

Dolby personalization Off and On

DTS personalization Off and On

The earlier DTS Off/On comparison used muted room sends. Version 2.0 restores them by default.

DTS Balanced and Spacious with identical personalized FIRs

The room comparison changes only the first native stereo auxiliary vectors. Personalized FIR bytes are identical. Across the researched directions, Spacious increased 20-250 ms energy by approximately 9.8 dB relative to Balanced. Original runtime, transaction counter, configuration, and device defaults were restored in the source experiment. Graph provenance records inputs and hashes without distributing the private recordings.

## Documentation and build

- User guide
- Technical report
- Release notes
- Project site
- Build and test guide

The application is C# / WinUI 3, unpackaged and self-contained. Build with .NET SDK 10, Windows SDK, and MSVC for the native helper/portable launcher. Native tests cover one Windows system; property readback and format checks do not establish acoustic output on every driver.

Release packages exclude personal profiles, raw captures, registry exports, endpoint identifiers, account data, and vendor preset/DLL files. Dolby and DTS names identify third-party products; this is an independent utility.

When moving from the separate 200% / 400% experimental build, use Restore there before applying a profile in version 2.0. Production and experimental builds keep separate restore records but share device audio settings.
