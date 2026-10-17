# Windows Spatial Audio Research wiki

This wiki is part of the repository so it is readable in a clone, a ZIP and GitHub's Markdown browser.

## Start here

- [Research overview](../README.md)
- [What works, what is unverified, and what is ruled out](../RESEARCH_STATUS.md)
- [Reverse-engineering methods](../docs/methods/README.md)
- [Research history](../docs/history.md)
- [Detailed research archive](../docs/ARCHIVE_INDEX.md)
- [Agent continuation guide](../agent-handoff.md)

## Topics

| Area | Guide |
|---|---|
| Platform contracts | [Architecture](../docs/topics/01-architecture.md) |
| Input before binauralization | [Object identity and interception](../docs/topics/02-object-input.md) |
| Replacing Sonic | [Native factory and full DSP](../docs/topics/03-sonic-renderer.md) |
| Dolby profile delivery | [Storage, integrity and endpoint use](../docs/topics/04-dolby-profiles.md) |
| Dolby room and HRTF coefficients | [Fields, room causality and synthesis limits](../docs/topics/05-dolby-room-and-coefficients.md) |
| DTS SOFA and room | [Personalized FIRs and native presets](../docs/topics/06-dts-personalization.md) |
| DTS PCM and authored objects | [Decoder routes and transport](../docs/topics/07-dts-decoding.md) |
| Recording a SOFA field | [Measurement and export](../docs/topics/08-sofa-measurement.md) |
| Sixteen-channel convolution | [Panning and actual APO host](../docs/topics/09-multichannel-convolution.md) |
| Speaker cancellation | [XTC](../docs/topics/10-crosstalk-cancellation.md) |
| Gaming/real-time constraints | [Latency and clocks](../docs/topics/11-latency.md) |

## Implementation and evidence

- [Examples and requirements](../examples/README.md)
- [Validation record](../VALIDATION.md)
- [Source index](../docs/SOURCE_INDEX.json)
- [Primary references](../docs/REFERENCES.md)
- [Release changes](../CHANGELOG.md)

The archived notes preserve technical depth and corrected mistakes. Their experimental claims are historical and scoped; the synthetic public examples have a separate fresh test record.
