# Dolby room processing and coefficient recovery

## The encoded directional field

The inspected profile contains a continuous polynomial directional transfer field in complex subbands, not a list of sampled time-domain HRIRs. The decoded logical field has shape `[ear=2, band=77, term=36, component=2]`. Thirty-six is a polynomial basis coefficient count; seventy-seven is a hybrid-band count. Neither is a number of HRIR taps or measured directions.

The basis contains degrees zero through five in a specific native order, with term counts 1/3/5/7/9/11. It is not automatically an ACN/N3D spherical-harmonic representation. Shared exponents, Q15 scaling, matrices, correction vectors, geometry, phase flags and increments must be preserved. Existing nonzero imaginary components make magnitude-only or arbitrary minimum-phase replacement destructive.

Native analysis bridges coefficient deserialization to source-gain construction and complex subband accumulation. That is stronger than identifying a serializer. Full sample-domain synthesis, scaling, delay, interpolation branches and final ear routing remain separate reconstruction contracts. The latest coefficient note still leaves the upstream complex-output-to-PCM synthesis bridge open. No pure recovered free-field HRIR is claimed from the coefficients alone.

## Important lane ranges

Zero-based ranges below are half-open unless explicitly stated. They describe the tested version, not every future format.

| Lanes | Meaning |
|---|---|
| 13 | Shared directional-field exponent |
| 1050–1562 | Room feedback coefficient region |
| 1578–1834 | Room output taps |
| 1834–1835 | Half sample rate |
| 1836–3132, 3132–4428 | Two 36×36 matrices |
| 4428–4500 | Two correction vectors |
| 4500–10044, 10044–15588 | Two complex directional fields |
| 15588–15619 | Phase flags/increments |
| 15619 onward | Geometry and shared trailer controls |

API lane `k` begins at `15 + 4k`; native low-word lane `k` begins at `2k`. Do not mix serialized API offsets, persisted-envelope offsets and offsets relative to native room/PCA structures.

## Why personalization changed room sound

The complete personalized tuning can change both direct and room behavior. Earlier paired captures showed a reduction in early/intermediate energy while later background remained essentially unchanged. That is a complete-tuning comparison, not isolation of a single room parameter. A short late tail does not prove a particular runtime branch was taken.

Static consumer analysis identifies paths that retain an initialized room/PCA pair and paths that substitute a built-in room structure while retaining directional data. Exact runtime branch choice requires observation; stored payload bytes and perceived dryness are insufficient.

## Room control methods

The research separately maps room output taps, feedback, mixing and room-path selection. Zeroing output taps can suppress the mapped room output while retaining directional fields. Restoring compatible room fields onto the current listener avoids silently replacing the listener with an old whole-profile baseline.

Scaling room taps by 1.5 predicts about +3.522 dB on that coefficient path, subject to quantization and signed overflow. It is not a calibrated vendor percentage or an acoustic RT60 result. An attempted two-times tap scale overflows the tested profile. Increasing feedback is not an interchangeable gain control because it changes recursive decay/stability.

Later application reports describe 200%/400% room/direct modes that attenuate the directional fields by 0.5/0.25 while retaining room coefficients. These ratios alter direct-to-room balance; they are not proof of doubled/quadrupled room energy, and no new acoustic measurement accompanies that label. Historical native readback/user tests and modeled coefficient effects remain distinct.

## SOFA recovery boundary

A renderer capture can measure the complete active binaural response even while coefficient-only synthesis remains unresolved. Conversely, an algebraically reconstructed directional field is not a measured HRTF. The research therefore keeps native virtual measurement, subband-field inspection and eventual SOFA recovery in separate logs. Full original filter tails, common ear timing and absolute gain are retained when making a measurement comparison.
