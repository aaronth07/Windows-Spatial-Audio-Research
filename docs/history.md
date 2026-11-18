# Research history

This timeline follows the research from early HRTF measurements to the release checks. Start with [Research status](../RESEARCH_STATUS.md) for the latest conclusions and their limits.

## HRTF/SOFA processing foundations

The earlier work separated measured channel captures from dense modeled direction fields. Seven directional anchors in a multichannel recording are not hundreds of independently measured HRTFs. Common timing and one normalization gain preserve relative ear/channel relationships; loadability in a consumer is not acoustic validity. Failed separation/factorization approaches and bounded listening candidates were retained instead of relabeling them as anechoic recovery.

Browser/offline multichannel processing work added complete sample/reference comparisons, common timing, cancellation/reset/error handling and explicit source-routing conventions. These practices carried into later native capture and convolution tests.

## Consumer PHRTF trace and endpoint use

The initial Access investigation followed storage filenames, typed configuration/deserialization and an uppercase-MD5 settings check. It did not establish a working consumer cache/UI import. Separate bounded native property experiments later produced reproducible candidate-dependent object-rendered output with baseline restoration. Those claims describe different boundaries and are not contradictory.

The profile application then brought together endpoint selection, typed profile loading, readback/rollback and separate Dolby/DTS room state. A later saved review found reset/state coupling and incomplete maintenance validation. Its warnings remain historical until a specific later source/test result supersedes them; publication does not infer that every application issue is fixed.

## Virtual object measurement

The capture project used native dynamic spatial objects and binaural recording to build SOFA fields. UI processing was moved away from capture/processing work. Export compatibility work corrected common transport/start alignment and restored useful response extent after aggressive short exports weakened front rendering. The tested short consumer export retained 4096 samples; long archival responses remained separate.

A generic standalone capture application was prepared in a separate project. This repository carries the research and native capture example rather than distributing that application's UI, account configuration or sessions.

## DTS personalization and room causality

Native profile conversion initially retained personalized directional filters but removed auxiliary vectors. Further tracing corrected the first vector set's interpretation to room sends. Compatible sends could be restored without replacing personalized FIRs. Named room presets were traced to installed assets, with compatibility checks and independent direct/late-window comparisons.

## Sonic replacement and decoding

Isolated native tests first established alternate DSP selection. Debugger analysis corrected an ABI field error. Later actual COM manifests and zero-GUID first-load behavior produced full-SOFA output through an unmodified engine. Full direction selection, preparation and safe early-entry behavior were added. Actual system-host admission and endpoint isolation remain unverified.

The available DTS media was decoded through spatial and ordinary PCM routes. The full ordinary route removed the need for a spatial metadata endpoint, but its extra channel columns were not additional authored objects. Whole-fixture prefix inspection and open-source decoder review separated channel-based DTS-UHD from legacy DTS object hypotheses.

## Object input, sixteen buses and XTC

Actual Windows forwarding tests retained source PCM/controls and then explicit multistream object lifetimes. The recorded input passed a native panner and the real Equalizer APO DSP against full independent references. The custom rendering was offline, so a live output application remains a separate integration task.

The XTC kernel retained full tails and source transitions against synthetic plants. Fusing static filters removed one buffering stage. Physical cancellation remains blocked by missing real four-path measurements, and tracker/plant motion remains distinct from virtual source rotation.

## Clocks and release snapshot

Real stream clocks and loopback source identification clarified that startup origins vary and API-timestamp-based latency is not acoustic arrival. The research handoffs were individually hashed and extracted-source examples rerun. This public release replaces private paths/data with anonymized notes and generated synthetic fixtures, then records a separate publication verification.
