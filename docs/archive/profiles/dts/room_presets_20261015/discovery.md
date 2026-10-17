> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# DTS Headphone:X room preset asset discovery

Static inspection of installed DTS Sound Unbound package version 2026.3.31.0. The package contains encrypted SAD assets under `Data/SAD/`, with named options including Gaming Balanced, Gaming Spacious, Gaming Spacious2, Movies Balanced, Movies Spacious, and Gaming Neutral. Their `.SPAC.crypt` files are equal in length (82,448 bytes) but have distinct hashes. The first 32 bytes are recorded as hex in `assets_manifest.json`; no decryption or payload parsing was performed. Equal lengths and distinct hashes do not establish the audio/DSP differences between presets.

The likely app-side selection surface is `DTSSoundUnbound2.dll`: static strings include `SelectedSADOptionIndex`, `AvailableOptions`, `OnSelectedSADOptionIndexChanged`, and `SADOptionsLocalizedStringConverter`. `DTSSoundUnbound2.xr.xml` declares `SelectedSADOptionIndex` and the converter (lines 11 and 163). `resources.pri` contains the Balanced/Spacious labels and `Data\\SAD` resource paths. This supports UI selection and packaged preset assets, but does not prove the runtime mapping from a selected label to a specific file or a call into the renderer SAD writer.

Copied app DLL, XAML schema, and the six named encrypted preset assets are in `artifacts/`. Their sizes and SHA-256 values are in `assets_manifest.json`; copied-file hashes were checked against source hashes. No vendor app was launched, no audio or configuration was changed, and no user LocalState data was inspected.

