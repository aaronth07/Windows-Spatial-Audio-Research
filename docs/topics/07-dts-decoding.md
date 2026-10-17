# DTS decoding and linear PCM transport

## The available fixture

The available media uses an MP4 `dtsx` sample entry and DTS-UHD framing. Every one of 2,816 access-unit starts was inventoried: 31 use sync word `40411bf2` and 2,785 use `71c442e8`. All 31 parsed sync-frame prefixes set the full-channel-based-mix flag. The alternate-frame syntax, complete profile conformance and CRC were not parsed.

This is an appropriate native channel-decode fixture. It is not a known authored moving-object trajectory. Fixed output coordinates or dynamic-typed speaker buffers cannot fill that missing ground truth.

## Two native output paths

The spatial MFT path produced ten float planes representing a 5.1.4 speaker layout, together with position metadata. It needed a metadata-factory selection, even though it did not itself start playback.

A separate ordinary PCM route decoded the complete clip without a spatial metadata endpoint or HDMI receiver. It produced twelve packed 24-bit columns. The front/center/LFE/heights correspond to the ten-plane reference; the two surround pairs are proportional duplicates. Sixteen-column output adds four zeros. This is ten independent signals, not twelve or sixteen original objects.

The complete run covered 2,883,584 frames at 48 kHz, approximately 60.0747 seconds. EOF, end-of-stream and drain were exercised, with exact conversion/queue sample accounting and expected final padding. Native packed output and the spatial float path have a small observed scale difference; fitted residual/correlation checks are separate from bit-exact transport checks.

## Transport implementation

The public headers implement signed little-endian 24-bit conversion and a single-producer/single-consumer ring. Construction allocates storage. Push validates finite input, extent, absolute source-frame index, EOF state and free capacity before publishing. Pop returns the number of valid source frames and zero-fills shortage. Silence insertion does not advance the source's absolute frame index.

The caller must decide what an underrun means: delay, drop or explicit resynchronization. Reset is a stopped-thread barrier and reports discarded queued frames. A fixed channel matrix determines output order; it is not object panning. Geometry and an object PCM+XYZ renderer are additional components.

## Open-source alternatives

The audited open-source decoder release has a legacy DCA/core/EXSS/XLL-X path with candidate extension/object feeds and PCM-plus-position events. This is a concrete alternative worth investigating, but parts of the geometry/fold interpretation are corpus-derived. Generated host feed IDs need not be original authored IDs.

The inspected route does not establish a DTS-UHD parser and requires a legacy core during actual decode. Its small shipped core fixture decoded fully in a released CLI. An information command incorrectly consumed a probe prefix and omitted an EOF frame; explicit codec selection and complete PCM verification distinguished that reporting bug from decoder failure. No locally verified moving-object corpus was available. A demux patch identifying a DTS-UHD sample entry also does not implement its object decoder.

## Latency

The source packet spans 1024 frames, or 21.333 ms. Average decode work below a millisecond does not remove packet arrival time, algorithmic delay, queue residence or device buffering. A local file can decode ahead and follow the device consumer. Independent live/network clocks need an explicit synchronization/resampling policy.

Demand and fixed-schedule queue models retain both passing cases and starvation under injected worker stalls. A bounded queue cannot guarantee playback under unbounded scheduling delay. Real device-clocked playback remains a separate experiment from faster-than-real-time file conversion.
