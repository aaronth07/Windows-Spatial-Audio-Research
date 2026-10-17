> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

Your SOFA-derived HRTF was applied and used by DTS Headphone:X through native Windows Spatial Sound dynamic objects. The bounded test completed six captures: original twice, converted twice, restored original twice. Dolby Atmos for Headphones and the saved endpoint configuration were restored afterward.

Test directory, capture analysis, restoration review, reviewed payload. The raw recordings and complete untrimmed responses remain beside each capture.

## Native-use evidence

The current installed encoder matches the analyzed binary SHA-256 `90e62a7f6d044b43e53413f1f0933647725838f60904de37cd4b49f3852a604c`. The selected/active DTS GUID was read back on the exact virtual endpoint before capture. The converter output was 754,556 bytes, SHA-256 `<private-artifact-digest>`; it replaces 22 directional FIR pairs on the installed native panner grid and retains the origin pair. No native byte patch, driver/service/security change, vendor-app launch or global-default setter was used.

Fresh Windows API runtime/device backups matched the inspected data. The actual saved counter was 4287, following the user's default-device selection; the approved write changed it to 4288. Setters returned successfully, with complete runtime/counter readback. The counter notification path was already traced to encoder reinitialization and FIR consumption. Runtime and counter restored to their fresh originals before the final capture pair.

Each run used the same 10 object positions, radius1m, one dynamic object, SoundEffects category,48 kHz mono float32 sweep at−34dBFS,250ms pre-silence and2s tail. Output was captured from selected virtual endpoint as stereo float32/48 kHz; all 10 directions completed in every run. Maximum recorded PCM magnitude was0.0474, and maximum pre-sweep RMS was about1.05e-5. The preserved samples lie on a16-bit quantization grid, consistent with the fresh persisted endpoint format; the float capture container does not add underlying precision.

Responses were deconvolved with the same inverse sweep. No normalization or separate ear alignment was applied. QPC transport correction found no direction-dependent buffer shifts. Comparisons use one common stereo correlation lag per complete field and the first 8,192 response samples.

| Comparison | Relative L2 response difference |
|---|---:|
|Original repeat|0.1478%|
|Converted repeat|0.1287%|
|Restored repeat|0.1509%|
|Original versus restored|0.1536%|
|Original versus converted|83.12%|

The converted/original difference remains75.96% even after fitting one diagnostic scalar gain, so a level change does not explain it. Original/restored energy differences are at most about 0.00022dB across the 10 positions. The restored response is close to the observed repeat-capture floor, rather than sample-identical;16-bit quantization and common stream timing remain present.

An additional coefficient-specific check learned one common complex transfer per ear from the converted front response, then predicted the side responses directly from the loaded FIR pairs. No side-specific delay, gain or filter was fitted. Over 300Hz–16kHz, median absolute magnitude errors were 0.0054–0.0272dB across the four side/ear combinations,90th-percentile errors 0.0164–0.0929dB, and complex relative L2 errors 0.164–0.540%. This correspondence, reversible profile-dependent differences and the static consumer path support actual use of the converted coefficients, beyond format or property acceptance. This check does not establish reproduction of every original SOFA direction.

## Restoration and notification state

The initial whole-registry comparison deliberately failed on one extra property and the initial runner returned a diagnostic error after the six captures. The original result is preserved. Investigation then identified that value in pinned`audiosrv.dll`, SHA-256 `<private-artifact-digest>`.

Function0x180124114 is named by its native diagnostic`SpatialAudioDevicePropertyWriter::SignalChange`. It writes `{<identifier-removed>},2` asVT_I2, combining the supplied reason byte with an incrementing sequence byte. Function0x1801382e0 independently writes the same key using that layout. The observed bytes changed from`02000000010000002103` to`0200000001000000eb61` after selecting DTS and`0200000001000000eb62` after restoring Dolby. This is Windows' notification state, not the selected format, profile, sample format or EQ. It was not rewound.

Native notification evidence, primary identity`ev_b0bf2d49d40195c0c2dacf5e651fab85eabecadee751b7c29617c65b1607c45f`. The updated runner distinguishes only this exact evidenced layout from configuration; malformed, missing or other changed properties still fail comparison. Synthetic comparison tests exercise those rejection cases, and the retained real snapshots reproduce the result without repeating audio captures.

All other snapshotted values match exactly: Dolby/DTS User and Default contexts, separate DTS device data, recording properties and other render properties. Original DTS runtime SHA-256 `<private-artifact-digest>`, counter 4287, and device SHA-256 `<private-artifact-digest>` were restored. Dolby profile registry SHA-256 remains`<private-artifact-digest>`, and independent getters returned the original Dolby selected/active format. The user's selected virtual endpoint default-device choice was retained.

The proof of concept remains version- and fixture-pinned. The DTS panner accepts at most32 positions; this candidate uses 22 directional positions plus the preserved origin. Native interpolation at arbitrary held-out directions, universal SOFA fractional-delay handling and a general user interface remain separate work. Personal/acoustic validity and anechoic conditions were not assessed. Windows Sonic import and independent Dolby room control are not completed by this DTS result.
