> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Dolby directional coefficient findings

The owned profile contains a continuous polynomial directional transfer field in complex subbands. It does not contain a measured direction list or a time-domain HRIR grid. Directional coefficients can be decoded and fitted without inventing phase. A pure recovered HRIR or free-field SOFA is not established by this work.

All work here was offline. No Dolby application, native DLL, audio endpoint, playback, capture, registry/property write, binary patch or configuration change was performed. The inert consumer is `DolbyAudioProcessing.dll`, SHA-256 `0fada40251880d03c657a357745d362799c93fb6024f733f1c8bf1057f5e4715`. Its static Ghidra 12.1.4 analysis used the independent `ghidra_projects/Coefficients` project. Imported PE dependencies were read as data, not activated. Analysis/cache files are Ghidra artifacts, not renderer state.

## Serialization and native fields

The profile SHA-256 is `<private-artifact-digest>`; candidate API payload SHA-256 is `<private-artifact-digest>`. Current inspection reconfirms all 15,691 signed-int32 coefficients and version `1.1.0` agree between source JSON and API payload, including extended checksum 1685. Inputs were reread and byte-identical after inspection. The inspected payload is the retained candidate, not a live property readback.

The source property is `personalized_hrtf.virtualizer_parameters.rosella_coefficients`. Retained consumer schema analysis identifies Int32[]; native `DecodeMessageTuning` narrows entries to low16 little-endian before rp deserialization. The API coefficient array begins at byte15; a registry envelope adds8 bytes. Thus API lane k occupies `15+4k,19+4k)`, while native rp lane k occupies `[2k,2k+2)`. See [consumer schema report and wire findings.

The fast rp deserializer `0x18010fd30` passes table B to `0x18010e9f0`, after assigning PCA header counts and the shared exponent. Native deserialization now independently corroborates the directional field layout. The following offsets are byte offsets relative to the PCA struct, not the room struct. Lane ranges are zero-based and half-open.

| Serialized lanes | Data | PCA field | Evidence |
|---|---|---|---|
| 13 | Shared field exponent2 | +0x150 | Fast rp parent passes it to PCA; decoder reads it for both fields |
| 1834..1835 | Sample-rate-half24000 | +0x158 | Decoder multiplies unsigned16 by2:48000 Hz |
| 1835..1836 | Matrix exponent | +0x138 | Decoder constructs float scale from exponent bits |
| 1836..3132 | First36x36 matrix | +0xf0 pointer |36 rows of36 signedQ15 scaled entries |
| 3132..4428 | Second36x36 matrix | +0xf8 pointer | Same decode |
| 4428..4464 | First36-term correction vector | +0x100 pointer | SignedQ15 with matrix exponent |
| 4464..4500 | Second36-term correction vector | +0x108 pointer | Same decode |
| 4500..10044 | First77x36 complex field | +0x110 pointer |154x36 serialized lanes,160x36 padded native floats |
| 10044..15588 | Second77x36 complex field | +0x118 pointer | Same decode |
| 15588..15608 |20 phase flags | +0x128 pointer, count+0x124 | Signed16 flags; decoder counts flags equal1 |
| 15608..15619 |11 phase increments | +0x130 pointer | OneQ15 float per flag1 |
| 15619..15624 | Five geometry scalars | +0x13c..0x14c | SignedQ15 |
| 15624..15691 | Shared geometry/distance trailer | Header/profile fields | Shared direct/room controls; not exclusively room |

The precise decoder is retained in native_coefficient_functions.txt. Signed values are scaled by `2^exponent/32768`; owned matrix exponent1 gives1/16384, and shared field exponent2 gives1/8192. The constant at `0x1803e4a88` is exactly1/32768. Native pseudocode sometimes prints bit reinterpretation as a numeric cast; it must not be translated literally as `(float)(exponent*0x800000+0x3f800000)`.

The independent inspector reconstructs `[ear=2, band=77, term=36, component=2]`. For serialized field-local index `i`, `band=i//72`, `term=(i%72)//2`, and `component=i%2`. The native four-band SIMD destination is `(i%288//72)+4*(i%72+72*(i//288))`; remaining80-band padding is zero. The two independent logical decodes match every component of the retained public parser with maximum error0. Each ear has720 nonzero imaginary components. Magnitude-only or minimum-phase substitution would discard existing encoded phase.

Both owned matrices are exactly36x36 identity after decoding. Consequently omission of matrix multiplication in the retained public direct evaluator makes no difference for this owned profile. It is not valid to generalize that omission to arbitrary profiles. The native metadata helper has a separate non-36 special path that reads those matrix pointers. Ear labels here follow the public parser; final physical left/right routing across all native paths still needs a complete endpoint/output bridge.

## Direction basis, gain and delay

Native `0x180041360` writes the same36 polynomial terms, in the same order, as the retained `direction_basis`. It includes degrees0..5 with term counts1/3/5/7/9/11. The constant terms were read directly from pinned file-backed PE bytes:1/3,3/5,1/5,1/15,3/7,3/35,1/35 rounded tofloat32. See native basis and geometry functions and native_constants.json. This is an explicit polynomial order, not an ACN/N3D-normalized spherical harmonic representation.

For example, the beginning is `[1,x,y,z,x²−1/3,xy,xz,y²−1/3,yz,(x²−3/5)x,...]`; the last term is `(y⁴−3/35)z`. There is no serialized sampled-direction order to recover:36 denotes basis coefficients, while77 denotes bands. A direction is evaluated continuously after metadata geometry mapping. Calling the coefficients77 directions or36 HRIR taps is incorrect.

Native metadata geometry `0x180041650` decodes position/control integers, applies profile mapping, axis scales and boundary clamps; `0x180041a40` computes ear-offset/correction basis vectors and a path difference. Its sample-rate multiplier uses a float32 constant equal `float32(1)/float32(343.3)`, one ULP different from converting the double reciprocal tofloat32. This is a small arithmetic difference, not established acoustic error. The retained public evaluator includes both geometric correction vectors, offsets, distance scale, equal-power weighting and central-source blending. Complete native arithmetic parity for those stages has not been established. Do not substitute a single fixed ITD or omit trailer/profile fields when modeling a direction.

Native metadata helper `0x18003e550` builds per-source gains from the decoded fields. `0x1800420d0` and `0x180042a10` then multiply those gains with complex audio subbands and accumulate two outputs. The selected arithmetic explicitly uses `(real*real−imag*imag)` and `(real*imag+imag*real)`. This bridges decoded coefficients to an audio summing stage, rather than merely identifying a serializer. Native direction and accumulation functions retain the complete selected bodies.

The phase generation in `0x18003e550` uses the20-entry count: flag2 holds the current phase, flag1 obtains a new trigonometric step from the next stored increment, and other flags continue the existing recurrence. Its selected complex-field path applies that phase to the first rounded-up20 bands and continues plain complex directional summation for subsequent four-band groups. It does not extend the final step to all77 bands. This supports the retained public model's unity additional geometry-phase factor after20 in this path. An automatic high-band continuation experiment is therefore not a justified recovery fix. Full branch equivalence, exact trigonometric ABI/argument recovery, dynamic gain interpolation and final physical ear routing remain unverified.

The public bank uses64-QMF to77-hybrid channels, splitting the first3 QMF bands8+4+4 and retaining61 high bands. At48 kHz the nominal high QMF spacing is375 Hz;20 phase entries extend through nominal2812.5 Hz. Those are architectural/public-model nominal values, not recovered native filterbank response peaks. The current payload's48 kHz field is proven; native analysis/synthesis kernels, impulse latency and phase-dependent reconstruction are not fully recovered here. A77-point IFFT is not a valid replacement for a hybrid filterbank.

## Fitting result and recovery boundary

The read-only fitting experiment samples400 Fibonacci-sphere directions from the decoded polynomial field, solves all36 coefficients by ordinary least squares, and evaluates257 separate holdout directions. Both fits have rank36 and condition number115.9296. First/second-ear maximum coefficient errors are2.0373e-14/1.7319e-14; maximum holdout real/imaginary component errors are1.0046e-14/8.7551e-15; relative holdoutL2 errors are4.1117e-15/2.7207e-15. These numbers quantify algebraic self-reconstruction only. They do not quantify Dolby-native rendering error, quantization error versus original measurements, or anatomical/acoustic accuracy. See inspection.json.

This establishes that the owned subband directional field can be resampled and fitted without numerical loss at practical floating-point accuracy. It does not identify an original free-field HRIR: analysis/synthesis filters, per-ear geometry, runtime mode, interpolation and gain controls contribute to the rendered transfer. Original measured HRTFs are not uniquely recoverable from a compressed polynomial/subband model without its analysis contract and source measurements. A model-rendered direct-only impulse could be useful after the missing synthesis and branch contracts are resolved; it must be labeled as modeled output, retain its latency/gain assumptions, and cannot silently become a measured or pure recovered HRTF.

Measured Windows renderer SOFAs remain renderer transfer measurements. Room-muted measurements could narrow the direct rendering path but do not by themselves remove filterbank, interpolation, endpoint or gain behavior. No SOFA was generated in this project.

## Artifacts and verification

Run `python DolbyCoefficientResearch/inspect_coefficients.py` for the default retained profile/API comparison and fit. It prints JSON and makes no writes. Run `python DolbyCoefficientResearch/inspect_native_constants.py` for pinned PE constant locations. Results saved for review are project-local; neither tool accesses live audio or native code execution.

Eight unittest groups passed: all11,088 decoded field components/layout endpoints, low16 signed conversion, unsupported shape/exponent rejection, independent random-field fitting, owned inputs/parity/preservation, CLI success/error behavior, source/API version mismatch and checksum rejection, and pinned native constant/identity checks. The source was reviewed again for native low16 semantics, API/registry coordinate separation, field dimensions, float32 precision, zero padding, interpretation limits and file-write boundaries. Ghidra reports completed analysis and complete selected function decompilation; runtime/perceptual checks are outside this offline scope.

The next concrete offline step is to trace native analysis/synthesis kernels and the metadata helper's callers, including gain interpolation and physical ear-output routing, then build an isolated mathematical reference that consumes the now-mapped PCA struct. Compare that reference against retained public code across directions, distances, profile modes and nonidentity matrix fixtures before producing any model-rendered HRIR. Native agreement cannot be reported without an independently supported reference or later separately authorized native comparison.
