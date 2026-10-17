# Research status

Snapshot: 2026-10-16. The evidence classes below apply to the particular experiment, build and interface, rather than every Windows implementation.

| Classification | Meaning |
|---|---|
| Verified in scope | Executed with the stated input, host and reference |
| Static finding | Supported by pinned shipped-code analysis; execution is not implied |
| Unverified | The required contract or runtime observation has not been established |
| Blocked experiment | A necessary fixture, measured plant, execution access or isolation condition is missing |
| Ruled out for a route/build | The examined interface or implementation does not provide the requested behavior |
| Impossible in general | A constructive information-loss argument establishes non-uniqueness |

## Current conclusions

**Object input:** actual documented spatial calls from an owned producer can be wrapped before EndUpdatingAudioObjects. Two streams, four simultaneous source slots and six explicit lifetimes were captured exactly. The forwarding layer is not a generic operating-system object-capture service. Asynchronous activation, unwrapped interfaces, arbitrary games and anti-cheat compatibility remain unverified.

**Sonic:** a private, unmodified native engine can create a custom full-SOFA DSP. The complete 710-bank/4096-tap direction field was exercised in one extension of the native test. The zero-GUID route also works when an owned CPU module wins first-load basename resolution. A property-configured bootstrap now prepares that module automatically. Native-first startup is detected as a missed preload opportunity. Safe early failure has been tested; real-host retry behavior has not.

**System insertion:** no installed system-wide replacement is claimed. Admission, process sharing, source-pool size, cross-engine dispatch, effect placement and resident-module restoration remain unresolved. Installation access was unavailable in the inspected process. This is not evidence that the broader goal is impossible.

**Multichannel:** available object PCM can be panned to an explicit speaker array. Native VBAP was checked at 4/8/16/24/32 channels, including 64 sources. Actual captured Windows objects pass a sixteen-bus panner and the installed Equalizer APO DSP with complete FIR tails. The custom output of this chain was evaluated offline. A live output FIFO, device callbacks and clock policy are still required.

**DTS:** ordinary multichannel decode and transport work for the complete available channel-based UHD fixture. Twelve PCM columns contain ten independent speaker signals; duplicated surround columns and added silent columns are not additional authored objects. A separate experimental open-source decoder has a legacy DTS object path, but locally verified moving-object coverage is missing. Its source does not establish DTS-UHD support.

**Personalization:** native endpoint-property delivery and UI/cache import are separate claims. Earlier consumer storage traces remained incomplete; later endpoint experiments support reproducible candidate-dependent processing. They do not prove the recovered coefficients equal an anatomical free-field HRTF. DTS room restoration preserves FIR bytes and changes compatible room-send vectors rather than replacing the personalized carrier with a stock profile.

**XTC:** processing, source changes and full tails work against synthetic plants. No physical speaker-to-ear cancellation result follows without actual four-path measurements.

## Negative results

- The ordinary post-mix APO and public WASAPI loopback contracts do not expose the original object IDs and coordinates.
- The inspected CPU Flex2 path ignores the legacy filter-bank interfaces and reaches embedded tables. Updating the legacy dataset file is not a working coefficient update for that route.
- Replacing only the private DSP factory does not change the earlier minimum-frame query in the inspected HrtfApo frontend.
- A sixteen-input/two-output graph was rejected by the tested stock APO format negotiation. A sixteen-column matrix with only its first two columns active is a different, verified host arrangement.
- Three nonzero source signals can produce identical zero stereo through known two-ear FIRs. Therefore arbitrary original object scenes cannot be recovered uniquely from their binaural mix alone.

## Publication verification

Archived notes record historical experiments. Public source adaptations and synthetic demonstrations are separately tested in [VALIDATION.md](VALIDATION.md). No historical acoustic, game, persistence or operating-system integration claim is silently promoted by a source build or passing synthetic test.
