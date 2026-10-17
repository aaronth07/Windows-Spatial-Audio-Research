> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Next output edge for future synthesis work

One bounded consumer-edge pass followed the established interleaved PCM Crossfade call to its success return. It found no additional direct call to the mapped QMF/hybrid routines in that edge. The missing synthesis bridge remains before Crossfade, between the two rendered subband channels and the consumer's PCM descriptor; this pass does not fill that gap.

The pinned consumer remains SHA-256 `0fada40251880d03c657a357745d362799c93fb6024f733f1c8bf1057f5e4715`. Work used the existing private Ghidra project and a read-only pseudo-disassembler. It did not rederive the coefficient layout, repeat the core+0xb0 displacement search, call the DLL, touch live audio/properties or synthesize an HRIR/SOFA.

## Immediate consumer edge

The prior instruction evidence establishes:

```text
0x180132cc3  LEA RDX,[RDI+0xb0]       PCM descriptor passed to Crossfade
0x180132cca  MOV R8D,[RDI+0x198]      sample-count argument
0x180132cd1  MOV RCX,[RDI+0x640]      Crossfade state
0x180132cd8  CALL 0x18011bd40
0x180132cdd  TEST EAX,EAX
0x180132cdf  JZ 0x180132e72          success edge
```

Function0x18011bd40 is identified by its own diagnostic as `dolby::oar::CrossfadeModule::Process`; the reviewed data loop uses interleaved float32 addressing `buffer0[sample*channel_count+channel]` and a common fade scalar. See ear_output_routing.md.

The existing Ghidra listing omitted much of the success continuation. The new read-only instruction dump decodes the exact PE bytes from0x180132e72 through0x180132fab without creating instructions or changing function metadata. The continuation sets the return status in ESI, follows lock/reference-count and allocator-shaped cleanup branches, walks a node list through0x180118580, then executes:

```text
0x180132fa5  MOV EAX,ESI
0x180132fa7  JMP 0x180133130
```

The earlier retained consumer instruction listing shows0x180133130 performing the stack-cookie check and returning at0x180133159. No mapped QMF analysis/synthesis, hybrid analysis or directional-field routine is directly invoked in the newly decoded continuation. The internal behavior of its cleanup-shaped callees was not expanded, so this is a statement about the selected direct instruction edge, not a universal absence-of-DSP proof.

## Exact remaining reconstruction gap

The established upstream result remains two complex subband output channels. The established downstream result is an interleaved PCM descriptor at parent+0xb0, processed by Crossfade and followed by the success-return path above. The pointer identity and transform sequence connecting those two results remain unproven: active hybrid recombination, the actual QMF synthesis dispatch caller, sample-rate-specific gain/normalization, common latency and phase-dependent delay behavior still need that bridge. Physical PCA-set0/set1 to Windows left/right PCM assignment also remains unresolved.

Therefore the downstream success edge does not provide a phase/delay or inverse-filterbank recipe for exact SOFA recovery. A77-point IFFT, the retained public bank's synthesis, a single fixed ITD, or an assumed removal of961 samples would each add an unsupported contract. The exact native analysis/synthesis selector and table facts already recorded in native_filterbank_findings.md remain useful inputs, but are not a completed inverse-renderer specification.

The primary separately reports approximately0.50013 output amplitude for half-scaled directional fields across10 directions and a room-fallback effect from trailer lane15689. Those are renderer-uptake and room-selection observations; they are not measurements of synthesis reconstruction error, phase accuracy, physical ear mapping or recovered HRTF validity. This pass neither reran nor independently recalculated them.

The one requested edge is retained and this pass is stopped. The blocker is the missing subband-to-PCM transform/alias contract upstream of Crossfade, not an unavailable coefficient array or an assumed absence of phase. No new waveform, nominal fixture or SOFA was produced.
