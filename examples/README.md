# Examples

The default suite is offline. Its filters, positions, PCM and profile words are synthetic, not captured listener data.

| Example | Purpose | Dependencies |
|---|---|---|
| `profile_format.py` | Bounded v2 wire parsing, low-word checksum and isolated room-tap proposal | Python standard library |
| `sweep_measurement.py` | Calibrated sweep/inverse, common stereo trimming and smoothed FR | NumPy/SciPy |
| `native/sofa_xtc_runtime` | Native full-tail object convolution and XTC primitives | C++17; Windows build script supplied |
| `native/system_object_input` | Actual private-engine COM/factory/bootstrap experiments | Windows SDK and compatible local native DLLs |
| `native/eqapo16` | Native speaker panner, fixed quantum and isolated actual-APO host | C++17; optional locally installed compatible APO |
| `native/dts_pcm_runtime` | Packed 24-bit conversion, SPSC transport and optional MFT source | C++17; native decoder/media supplied separately |
| `native/live_object_tap` | Single-stream forwarding of actual Windows object inputs | Existing spatial endpoint; explicitly invoked |
| `native/live_object_tap_multistream` | Separate stream/object lifetime capture | Existing spatial endpoint; explicitly invoked |
| `native/live_object_clock` | Actual spatial stream clock services and offline analysis | Existing spatial endpoint for capture |
| `native/live_object_latency` | Independent PN sources, loopback timing and offline identification | Existing stereo float32/48 kHz endpoint for capture |
| `native/open_object_decoder` | MP4 access-unit and DTS-UHD prefix inspection | User-supplied media; Python standard library |

## Default synthetic suite

From the repository root:

```powershell
python -m unittest discover -s tests -v
python tools/generate_fixtures.py
python tools/run_examples.py
```

Generated fixtures go to ignored `fixtures/generated`; the default runner uses temporary result directories. Existing explicit output paths are refused. The runner builds native sources on Windows and opens no device stream. Installed-library compatibility requires separate probes.

The public runner does not require a personal SOFA or media file. A generated SOFA is a numerical fixture only. Its receiver geometry and synthetic filters are not anatomical measurements.

## Native engine experiments

The private engine probes accept an explicit directory containing compatible local HrtfApo/HrtfDspCpu files. They verify reviewed hashes before using private interfaces. AudioEng provider tests additionally require the reviewed AudioEng DLL. A current but different Windows build may fail that compatibility check; do not disable the pin and assume the same ABI.

The bootstrap's prepared container is an example-owned format, not a vendor file format. It contains `WSSSOFA1`, version/mode/dimensions and a SHA-256 payload digest. A mode-2 payload carries complete ear filters and Windows-coordinate direction vectors. The initializer prepares outside render callbacks. The digest detects corruption; it does not authenticate a publisher.

The actual Equalizer APO host takes a DLL, separate configuration directory, PCM input, new output and metrics paths. It redirects only its private image's reads. Never use the installed master configuration as the research output directory. Exact build pins and format/quantum limits remain part of the experiment.

## Live source capture

These commands are optional and render low-level synthetic audio. Select a spatial provider yourself before running them. They use the current default multimedia render endpoint; no machine-specific name or ID is embedded.

```powershell
pwsh -NoProfile -File examples/native/live_object_tap_multistream/build.ps1
examples/native/live_object_tap_multistream/tap_host.exe examples/native/live_object_tap_multistream/producer.dll runs/live-capture
python examples/native/live_object_tap_multistream/verify.py --help
```

The host records PCM and object metadata locally; generated runs are ignored by Git. The adaptation omits the endpoint instance ID from the written result. Missing dynamic objects or unsupported formats produce a diagnostic rather than a provider/default/format change. The wrapper still forwards native audio: it is not a complete live custom-renderer output application.

Capture from an arbitrary game, system-wide installation, device-clocked custom output, physical XTC and acoustic validation remain outside the default examples. See [research status](../RESEARCH_STATUS.md).
