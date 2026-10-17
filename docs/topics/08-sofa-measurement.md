# Virtual SOFA measurement

## What is measured

A stationary native dynamic Windows spatial object can act as a virtual source. For each fixed direction, render a known excitation and record the selected endpoint's stereo mix through WASAPI loopback or an explicitly selected recording endpoint. Deconvolution yields the complete active source-to-recording response, including the renderer, endpoint effects and capture path.

This uses the object interface rather than a finite channel-bed impulse test. It can sample hundreds of directions, but a dense grid does not establish that the response is anechoic or that the provider's personalization is anatomically correct. A SOFA convention declares a data interpretation; it does not prove how the data was acquired.

## Acquisition records

Retain the exact source grid, stimulus/inverse, sample rate, pre-silence, response tail, submission/update records, render/capture timestamps, discontinuity flags, continuous recording, completed direction ranges, quality checks and processing version. Without those records, an apparent source delay can be a transport offset or an incomplete capture.

The historical application separated Win32 UI, worker orchestration and native capture so processing did not freeze the UI. Pause/Stop, incomplete direction sets and export failures need defined behavior. A nominal 710-point sphere and an explicit latitude/longitude grid are different sampling designs; neither should silently be substituted for another in a comparison.

## Swept-sine processing

An exponential sine sweep and a calibrated time-reversed, exponentially weighted inverse give a repeatable linear-response estimate. Self-convolution calibrates scale and reference offset. Nonlinear harmonic responses and time variance require separate treatment. One direction should use one excitation and one stereo recording with preserved channel order.

Do not normalize each ear/direction independently. Preserve relative ear timing, interaural level differences and common absolute level. A shared stereo onset reference is different from shifting each ear's peak to zero.

## Trimming and compatibility

Short exports initially sounded weak at the front. The repair distinguished timestamp-derived whole-buffer offsets from acoustic/renderer response timing and retained more of the response. A 4096-sample export worked in the tested consumer tool; a longer export did not. Those are observed compatibility limits, not a universal SOFA maximum.

Trim to a defensible response start with one shared stereo reference, then retain the requested sample extent. A short final fade avoids a discontinuity; it must not remove substantial early energy. Keep a longer archival response separately. External Data.Delay and embedded delay must not be double-counted.

Receiver coordinates used by the virtual measurement are nominal metadata, not recovered anatomy. GeneralFIR can preserve a complete response while SimpleFreeFieldHRIR is used for compatible binaural consumers. Downstream acceptance, correct metadata, full tail handling and actual acoustic interpretation are different tests.

## Comparisons

Use common frequency axes, absolute levels, identical windows and consistent one-sixth-octave smoothing when comparing modes. Room decay, impulse response and frequency response answer different questions. A late-energy difference is not automatically RT60; a trustworthy estimate requires usable decay range above the measured floor. A background-limited tail should not be fit as reverberation.

The public examples use synthetic responses only. They demonstrate timing, gain and file-format mechanics without publishing an individual's measured field. The native capture source is included as a research implementation; the full application UI and private sessions are not part of this repository.
