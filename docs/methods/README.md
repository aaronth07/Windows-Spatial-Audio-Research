# Reverse-engineering methods

Choose a method based on what you want to verify: which code is present, which path runs, how the data is structured, or what changes at the output. Each method answers a different question.

| Method | What it establishes | Important limit |
|---|---|---|
| PE identity, versions, imports/exports/resources | Which shipped artifact and entry points were inspected | A matching string is not an executed path |
| REA/Ghidra cross-references and decompilation | Candidate control/data flow, structures, ownership and constants | Optimized fragments, jump tables and inferred types need instruction checks |
| Raw instruction/exception-range analysis | Real call sites and enclosing function boundaries when decompilation fails | Does not alone establish runtime branch selection |
| Managed/AOT schema and serialization trace | Typed arrays, option precedence, stored settings and integrity computation | Consumer storage is not necessarily the native audio consumer |
| WinDbg/CDB owned-process probing | Actual ABI values, module selection, crash context and call behavior | A private host is not audiodg or an arbitrary game |
| COM manifests and forwarding wrappers | Actual factory activation and source retention at a defined seam | Interface coverage and identity/lifetimes remain explicit |
| Isolated import redirection | Configuration isolation or activation wrapping in an owned image | Does not establish operating-system-wide interception |
| Differential format edits | Which byte/word fields change, with repaired checksums | Native acceptance and audible effect require separate evidence |
| Independent convolution/panning references | Numerical processing, full tails and control transitions | Not anatomical or physical validity |
| Native object sweeps/loopback | Complete active source-to-recording response | Includes provider/postmix/device processing |
| Clock correlation and independent PN | Relative timing and identifiable payload/output signatures | API timestamps are not independently calibrated acoustic arrival |
| Paced/queue/stall tests | Specific budget, starvation and clock hypotheses | No guarantee under unbounded load |

## Static workflow

1. Pin the input artifact by SHA-256 and version, then retain the analysis tool/version and profile. Record addresses as VA or RVA explicitly.
2. Trace a specific feature from entry point through deserialization, state selection and consumption. Keep positive and bounded negative searches separate.
3. Infer a structure from multiple accesses, callers and allocation sizes. Distinguish API/persisted offsets from native object-relative offsets.
4. Inspect instructions when pseudocode depends on a type guess, bit reinterpretation, unresolved jump table or secondary function fragment. Use the PE exception directory and file-backed constants as independent anchors.
5. Preserve evidence IDs and coverage limitations. A trace ending at an indirect call is an open edge, not a verified consumer.

The Dolby consumer Process path was initially absent from a decompilation because of an unresolved jump table. Raw diagnostic/call scans and exception-range bounds found the continuation. A float exponent constructor also looked like a numeric cast in pseudocode; treating the bits as an ordinary cast would have produced wrong scaling.

## Runtime workflow

Prefer a new owned host with inert copies or explicit installed-library paths. Pin DLLs before calling private RVAs. Use synthetic tagged PCM before real filters: channels, source slots and frame extents should be independently recognizable. Capture all HRESULTs, written lengths, guards, cleanup and module ownership.

One failed Sonic probe interpreted a frame-count field as stride. The debugger identified the invalid read and the corrected contiguous layout passed. One DTS private ABI selector crash remains quarantined: an unsupported call/lifetime is not proof of a vendor defect or an impossible decoder goal.

A useful CDB session records the exception context and disassembly, not only the top symbol. When symbol resolution prints a nearest export, label it as such. For call tracing, break on a pinned module-relative address in the owned host, inspect the connection/property record and preserve the command/output. Do not substitute a loaded module or a successful CoCreateInstance for an actual custom Render marker.

The published examples use process-local manifests and import edits only in their owned modules. They contain no system registration or service-restart workflow.

## Format and coefficient workflow

Parse bounded optional values, integer widths, signed low words, version strings and exact extents. Compare API and persisted envelopes independently. Identify checksum coverage by recomputing it on the smallest isolated field edit; MD5, XOR and SHA-256 have different roles.

For room experiments, freeze directional fields and edit only the mapped sends/output fields. For directional reconstruction, preserve complex components, shared exponents, matrices, basis order, phase and geometry. Algebraic self-reconstruction is not native arithmetic parity. Native deserialization is not final audio synthesis.

The public profile example handles only the evidenced v2 layout and offers an in-memory tap proposal. It does not accept unsupported overlapping representations by guessing and does not deliver candidates to an endpoint.

## Causal measurement workflow

Keep the exact immediate baseline and state identities. A repeat/current/candidate/restored-current sequence tests repeatability and recovery separately. Preserve the source grid, excitation, inverse, capture path and full responses. Use one common stereo alignment and gain reference; independently shifted/normalized ears can manufacture a difference or erase one.

Absolute early/intermediate/late energy windows, floor estimates, impulse plots and smoothed FR answer different questions. Fit a room-decay slope only over a justified range above the floor. State changes, stream restart uptake and audible results need separate observations.

## Negative results and publication

Retain failed cases with the reason, corrected hypothesis and changed input. Do not rerun a crash without a new hypothesis. A parameter rejection, absent fixture, access failure or unsupported service is route-specific evidence.

Public notes omit private data and fixture-bound commands. Their original experiments were not rerun merely to export documentation. The synthetic public runner records its own numerical/build results. [Current status](../../RESEARCH_STATUS.md) separates those scopes.
