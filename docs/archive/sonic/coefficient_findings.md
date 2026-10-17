> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Sonic coefficient findings — 2026-10-15

Separate coefficient/SOFA recovery record. These findings do not establish a working DefaultHrtfs.bin replacement or an installed custom renderer.

Pinned HrtfDspCpu SHA-256: `4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9`. Retained prior static analysis under `SpatialRendererProfileImport/windows_sonic` identifies an embedded spatializer with 16-weight directional interpolation. The selected default factory does not consume its supplied directional-filter/filter-bank interfaces.

| Descriptor RVA | Getter RVA | Data RVA | Retained interpretation |
|---|---|---|---|
| 0x1423e8 | 0x68d0 | 0x1b2df0 | 360 × 181 signed-16 directional lookup |
| 0x142388 | 0x68f0 | 0x172970 | 16 × 4,096 float table |
| 0x142320 | 0x6910 | 0x152430 | 16 × 1 float table |
| 0x142348 | 0x6930 | 0x172470 | 20 × 16 float table |
| 0x142368 | 0x6950 | 0x1b2bf0 | 16 × 8 float table |
| 0x142408 | 0x6970 | 0x1b29f0 | 8 × 16 float table |
| 0x1423c8 | 0x6990 | 0x152470 | 2,048 × 16 float table |
| 0x1423a8 | 0x69b0 | 0x1b2970 | 16 × 2 float table |
| 0x142300 | 0x69d0 | 0x142430 | 8,192 × 2 float table |

The float tables are not yet assigned a complete filter/transform interpretation. Do not export them as direct HRIR samples on shape alone. Native factory/render execution confirms that the pinned CPU implementation produces directional impulse responses, not which table contains a directly recoverable time-domain HRIR basis.

The final isolated native outputs are retained under `validation/20261016T035319038Z/sonic_dsp`. Front/left/right inputs are (0,0,−1), (−1,0,0), (1,0,0). Each input is a 0.01-amplitude impulse at frame 3,072 with source gain 1 and directional blend 1. Output is interleaved float32 stereo, 12,288 frames at the implementation's 48 kHz context. The real source frame-count field is offset 112, not a stride. Corrected probes were rerun after this ABI finding.

Each response's largest peak is 168 samples after the input. Threshold onset is 0–2 samples after input, depending on ear/direction. These are filter-response measurements in an isolated fixed-block call, not total playback latency or proof of anechoic response. Left/right ear-energy asymmetry reverses with source side. No per-ear normalization was used.

Future recovery should sweep a fixed full-sphere grid through the verified native boundary, retain common timing/gain, and distinguish measured renderer impulse responses from direct serialized-coefficient recovery. Personalized PHRTF/DTS coefficient work remains in its existing separate logs.
