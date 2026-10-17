# Windows audio architecture

## The three different jobs

A codec decoder turns a compressed bitstream into signals and, where supported, scene metadata. An object renderer accepts source PCM plus controls such as position and gain. A binaural renderer filters or spatializes those sources into two ear signals. A speaker renderer distributes them into output channels. These jobs may be implemented in the same product, but they are not interchangeable API contracts.

Windows Spatial Sound's application-facing sequence is based on PCM objects. A client activates ISpatialAudioClient from an IMMDevice, selects a supported mono format, activates a render stream and obtains objects. Its update transaction supplies mono sample buffers and object controls. This does not itself provide an Atmos, DTS or general compressed-media decoder.

The system can choose a native spatial provider/renderer. An APO later processing an endpoint's PCM has a different view of the signal from the spatial object's producer. Seeing multichannel buffers in an APO does not establish original object identity, position or authored trajectories.

## Interfaces and boundaries

| Boundary | Data established here | What must not be inferred |
|---|---|---|
| ISpatialAudioObject input | PCM and explicit object controls | A compressed codec stream |
| Native private HrtfApo engine | Separate sources and a DSP factory seam in the tested build | Public renderer registration |
| AudioEng optional provider wrapper | Dynamic descriptor, PCM, metadata/context and output channels in direct calls | Complete discovery/activation/lifecycle |
| Public APO RT connection | PCM pointer, valid frame count and buffer flags | Original scene objects |
| WASAPI loopback | Current endpoint render mix and packet timestamps | Dry source isolation or original object recovery |
| SOFA file | Declared FIR/delay/position data under a convention | Proof of anechoic acquisition or anatomical accuracy |

The ordinary public APO path may process multichannel PCM very usefully. It simply does not acquire information absent from that PCM interface. Likewise, a loopback measurement can characterize the complete active renderer and endpoint path without exposing that path's internal source scene.

## Static and dynamic sources

Static object types identify named speaker roles; dynamic objects accept three-dimensional positions. A dynamic-typed buffer can still represent a fixed speaker. The available DTS fixture exposed constant-position dynamic-typed data, so that classification alone did not establish original moving authored objects.

Windows object positions use right/up/back axes: positive X is right, positive Y is up, negative Z is forward. A common SOFA Cartesian convention uses front/left/up. The conversion used in these examples is `SOFA = (-WindowsZ, -WindowsX, WindowsY)`. Direction vectors, handedness, units and listener rotation must be specified at every boundary.

## Implementable routes

An application owning its PCM and coordinates can render directly through a custom engine. A forwarding layer can retain those inputs immediately before Windows receives the update. The private native factory can also select a custom DSP in an owned process. A system-wide replacement additionally needs admission, first-load control, host scope and a restoration strategy. None of those last requirements follows from constructing a COM class in a test process.

See [object input](02-object-input.md), [Sonic replacement](03-sonic-renderer.md), and the [current status](../../RESEARCH_STATUS.md).
