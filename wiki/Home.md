# Windows Spatial Audio Research wiki

Use this index to find a topic guide, a runnable example, or the evidence behind a result. The same pages are available in the repository and its source archives.

## Start here

- [Research overview](../README.md)
- [Research results and limits](../RESEARCH_STATUS.md)
- [Run the examples](../examples/README.md)
- [Reverse-engineering methods](../docs/methods/README.md)
- [Research history](../docs/history.md)
- [Detailed research archive](../docs/ARCHIVE_INDEX.md)

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

- [Validation record](../VALIDATION.md)
- [Source index](../docs/SOURCE_INDEX.json)
- [Primary references](../docs/REFERENCES.md)
- [Release changes](../CHANGELOG.md)

Use the validation record for checks on the public examples. The archive records the original experiments and subsequent corrections.
