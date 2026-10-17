# Object input, lifetime and interception

## Submission sequence

The producer activates a real spatial render stream, starts it, waits for its event, calls BeginUpdatingAudioObjects, acquires object buffers, sets position/volume, and finishes with EndUpdatingAudioObjects. Data must be retained before End: the underlying buffers are not owned by the observer and cannot be kept as long-lived pointers afterward.

The example forwarding host wraps an ordinary producer DLL's CoCreateInstance import in its own process. It wraps the returned enumerator, device, spatial client, stream and objects while forwarding operations to the real Windows interfaces. It copies PCM/control records immediately before forwarding EndUpdatingAudioObjects. The producer does not know the capture buffers or wrapper IDs.

This is stronger input evidence than a mock spatial client, but narrower than intercepting arbitrary games. A different activation path, unwrapped QueryInterface result, asynchronous activation, HRTF stream variant or COM apartment model can bypass the tested wrapper.

## Stable identity

Object identity is a lifetime property, not a position, raw pointer or PCM address. Buffers and addresses can be reused. The multistream example assigns a stream ID and monotonic object ID; a slot generation changes when an object is released and recreated. A panner resets that lifetime's pan/gain history. Previously submitted audio keeps its original convolution tail.

The recorded two-stream experiment had four simultaneous source slots and six lifetimes. A continuing object retained its ID while another object was ended, released and recreated. Metadata and PCM matched the producer reference exactly. These captures are not distributed; the code and anonymized result boundaries are.

EOS is also not identical to Release. The tested producer submits a final valid-frame count, drains update transactions until the object reports inactive, then stops/resets and releases. A general adapter must also retire an object released without EOS and handle Reset, stream replacement and disconnect. The current private engine proof uses fixed source slots until engine release and does not inherit the public wrapper's complete lifetime semantics automatically.

## Integration point

One callback per update is preferable to one callback per object. A bounded array can expose object ID, type, const PCM span, valid frames, position, volume and EOS. A synchronous observer can render once, copy into a prepared FIFO and return. The forwarding End call must still occur if observer processing fails; capture failure and native forwarding failure need separate results.

The observer must not retain the source pointers, allocate unbounded storage or perform file/network work in the update interval. A fixed-size pool fails explicitly at capacity rather than silently changing identity.

## Duplicate audio and clocks

Retaining input while forwarding it means Windows still renders the native stream. Sending the custom result to a device without a deliberate native-output policy duplicates audio. The published examples demonstrate forwarding/capture; they do not quietly mute an unrelated stream or install a new endpoint route.

Two streams started close together do not necessarily have identical progress origins. The capture comparison aligns producer sample zero for an offline numerical reference. Live output needs a separate clock relationship and discontinuity policy. See [latency](11-latency.md).

The native examples now resolve the current default render endpoint rather than containing a machine-specific device name. Capability failure is reported; no provider or endpoint format is changed.
