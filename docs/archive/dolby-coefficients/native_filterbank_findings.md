> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Native filterbank and metadata follow-up

Static analysis now identifies the consumer's QMF analysis and synthesis routines, the 48 kHz coefficient tables, and the 13-tap low-band hybrid analysis. It also establishes two metadata-update callers and a 256-sample update cadence in one selected wrapper. Complete native PCM reconstruction, synthesis call-site uptake, dynamic interpolation equivalence and final Windows physical ear routing remain unresolved.

The consumer remains pinned to SHA-256 `0fada40251880d03c657a357745d362799c93fb6024f733f1c8bf1057f5e4715`. All functions below were inspected in the independent Ghidra 12.1.4 project `ghidra_projects/Coefficients`. No DLL execution, playback, capture, property/registry writes, profile changes or SOFA generation occurred. All retained outputs and extracted tables remain private and ignored by Git.

## QMF dispatch and tables

Initializer `0x180004d40` selects configuration through `0x180011670(sample_rate)`. At 48 kHz the lookup returns static descriptor `0x1802ea1a0`. The initializer copies its byte +0x10 pointer into core +0x90, +0x18 into core +0x98, and +0x20 into core +0xa0; it installs `0x180012010` at core +0xa8 and `0x180011970` at core +0xb0. This is an explicit native dispatch assignment, not inferred from a familiar coefficient shape. Initializer and dispatch, configuration lookup.

The 48 kHz descriptor contains:

| Descriptor byte offset | Value | Selected consumer |
|---|---|---|
| +0x00/+0x04 | 48000/48000 | Sample-rate configuration |
| +0x08 | 256 | Block/control dimension; initializer selects 64-band processing for this value |
| +0x10 | 0x1802f0080 | QMF polyphase coefficients, core +0x90 |
| +0x18 | 0x1802efe80 | Complex QMF modulation coefficients, core +0x98 |
| +0x20 | 0x1802efe60 | Hybrid low-band descriptor, core +0xa0 |

Native `0x180012010` maintains nine previous 64-sample blocks and incorporates the current block. Its four-lane loop advances 40 coefficient floats per four input lanes, giving 64 x 10 floats. It computes alternating even/odd polyphase sums, complex modulation, a size-selected transform through `0x180088590` or an installed transform object, and real/imaginary output reordering. The producer wrappers `0x18002d690` and `0x18002e940` call core +0xa8 with band/hop parameter 64 and four slots. Their internal 64-band buffers use 0x200 bytes per complex slot; the 77-band buffers use 0x280 bytes, including SIMD padding to 80. Complete analysis function, producer callers.

Native `0x180011970` reverses the input component ordering, calls the size-selected transform through `0x180088170` or the transform object, applies complex modulation, updates its polyphase synthesis state and writes each output value with a final factor 2. It consumes a 40-float coefficient group per four lanes, as the analysis routine does. Its complete body is retained in native_qmf_dispatch.txt. The transform functions' exact normalization and complete synthesis call-site chain have not been proven; the final factor 2 must not be interpreted independently as overall PCM gain.

A bounded decoded-instruction scan found the two indirect analysis calls at `0x18002dbb5` and `0x18002ebf1`. It did not find a direct computed call with displacement +0xb0. This does not show that synthesis is unused: the pointer may be copied and called through a register or another dispatch object. A second bounded displacement-read scan also did not resolve that chain. Indirect calls, pointer reads. The distinction between installed synthesis dispatch and observed synthesis uptake is retained.

## Hybrid low bands

The descriptor at `0x1802efe60` has:

- +0x00 pointer `0x1802efd60`: 13 complex float32 coefficients for the 8-band branch.
- +0x08 pointer `0x1802efde0`: 13 complex float32 coefficients for both 4-band branches.
- +0x10 count 13; +0x14 delay/control value 6.

Function `0x180013220` explicitly reads this count and the two coefficient pointers. It keeps three complex input-band histories, accumulates them with the two complex coefficient vectors, invokes transforms of sizes 8, 4 and 4, then reorders/sign-adjusts 16 low output bands using the table at `0x1802edc00`. The high QMF channels remain separate in the producer's 77-band buffer path. This supports a native 64-to-77 hybrid analysis bridge: replace three QMF bands by 8 + 4 + 4 children, retain 61 high bands. Complete hybrid function, selected producer call.

The 16-entry native low-band order is `[0,7,1,6,2,9,3,4,10,5,11,15,8,12,13,14]`. Order and sign changes are part of the transform contract. The two tap tables contain interleaved real/imaginary float32 values; neither is a time-domain HRIR. The descriptor's value 6 and the 13-tap history support a six-slot low-band delay, but do not by themselves establish total PCM latency or native agreement with the public bank's 961-sample compensation. Native hybrid recombination/synthesis has not been completely traced in this pass.

## Retained table bytes and comparison limits

The following exact file-backed ranges were extracted to native_tables_48000. Each manifest entry records virtual address, file offset, byte count and SHA-256; the binary sidecars are the exact corresponding PE bytes:

| Table | Virtual address | Byte count |
|---|---|---|
| 48 kHz configuration descriptor | 0x1802ea1a0 | 192 |
| QMF polyphase coefficients | 0x1802f0080 | 2560: 640 float32 values |
| QMF modulation coefficients | 0x1802efe80 | 512: 128 float32 values |
| Hybrid descriptor | 0x1802efe60 | 24 |
| Hybrid 8-band coefficients | 0x1802efd60 | 104: 13 complex float32 values |
| Hybrid 4-band coefficients | 0x1802efde0 | 104: 13 complex float32 values |
| Hybrid output order | 0x1802edc00 | 64: 16 int32 values |

The extractor verifies the complete pinned consumer hash before interpreting sections. Every range must lie wholly in exactly one file-backed PE section. Default inspection prints JSON without writing; optional `--output` requires a new directory inside this private coefficient project and refuses overwrite. Extractor, table values and locations.

A candidate four-lane tap unpacking was compared to the retained public `qmf_analysis_coefficients`. It does not directly match under a common scale with or without an alternating tap sign: the latter candidate gives relative coefficient L2 difference 0.4499 after best common scaling. This is a rejected simple mapping hypothesis, not native reconstruction error. Full lag ordering, modulation, transform normalization and conventions were not reconciled. The native synthesis body's polyphase state processing also cannot be replaced by the public bank's independently constructed causal left inverse merely because both have 64 bands. No claim of public/native filterbank equivalence is made.

## Metadata updates and output routing

Metadata evaluator `0x18003e550` has two direct native callers: `0x180025a80` and `0x180018bc0`. Selected complete callers and references are retained in native_filterbank_pivots.txt and native_filterbank_callers.txt.

In `0x180025a80`, loop index `uVar37` advances over the block's subband slots. It calls `0x18003e550` when `(uVar37 & 3) == 0`, advances accompanying sample counters by 0x40 per slot, and calls direct accumulator `0x1800420d0` every slot. Therefore this selected 64-sample-bank route refreshes metadata/gains every 256 input samples. Metadata timing arguments, transition limits, mode-dependent control and initial state can alter the effective trajectory; this is not a universal API update promise. The other wrapper `0x180018bc0` calls `0x18003e550` before its slot loop and carries additional state/control updates within the loop.

The second wrapper uses helpers `0x180018300` and `0x1800186b0`, which compute bounded scalar limits from prior/current parameters with factors 1/32, 32, 1/256 and 256, radius-like square roots and clamped results. Their complete bodies are retained in native_interpolation_functions.txt. This identifies native parameter transition/control code rather than a static per-direction lookup. The complete field identities, time units, transition semantics and audio-rate gain interpolation have not been reconstructed, so a simple linear interpolation policy is not substituted.

The direct accumulator produces two distinct output pointers. In `0x180025a80`, selected branches copy its first and second outputs into the first two channel buffers of the receiving subband block, then set the block's channel count to 2. Other branches use additional output/room buffers. Decoder ear fields, delayed-ear selection and temporary argument swaps must be followed as a pair; argument position alone is not a physical ear label.

The room-path agent's separately retained `0x180010040` wrapper reaches the producers `0x18002d690` and `0x18002e940`. The selected outer path also converts sample-format descriptors using `0x18002fa10`. This connects the native metadata/complex accumulation to consumer block processing, but no complete Windows channel-mask/output-array bridge establishes physical left/right ear identities for every route. No source-backed endpoint routing label was invented. Final physical ear routing, the active synthesis chain and the Windows caller of the raw processing wrapper remain bounded next targets.

## Verification and next step

All 12 offline test groups pass, including exact PE byte extraction, 640/13-complex table lengths, consumer hash and file-backed-bound checks, CLI/export paths, refusal to overwrite existing outputs, outside-directory rejection, complete owned field parity, fitting and input preservation. The native text outputs were checked for completed selected bodies and explicit missing selectors. The speculative selector `0x180010070` is marked `MISSING` and supplies no evidence; it was not treated as a function. No table bytes or DSP algorithms were modified.

The next concrete static step is to follow core +0xb0 through register/object dispatch to its actual synthesis callers, then identify the hybrid recombination and trace the two resulting PCM channel pointers through the named module/Windows output bridge. That is narrower than implementing a replacement bank. After this contract is complete, an offline reference can use the extracted native tables and retained transform/scaling order; until then neither a 77-point IFFT nor a modeled public-bank impulse qualifies as a recovered native HRIR.

## Bounded output def-use pass

A further bounded pass followed decoded output-side instructions, rather than repeating the displacement scan. It retained the complete outer process body `0x180010040`, the producer return paths after their calls to core `0x180007d10`, and the parent consumer output instructions. The synthesis dispatch edge still did not resolve, so this search stops here.

At `0x18002f757`, producer `0x18002e940` calls the core. It then constructs a stack-local format descriptor and calls converter `0x18002fa10` at `0x18002f7a6`. The apparent +0xb0 pointer read is demonstrably unrelated to core +0xb0: `0x18002f77e` sets RCX to `RSP+0x78`, and `0x18002f78e` loads RDX from `[RCX+0xb0]`, hence from `RSP+0x128`. It supplies the converter's destination argument. The other producer's output path similarly uses `LEA RCX,[RSP+0x50]` followed by `[RCX+0x158]` as its stack-local destination argument. Neither is a transfer of the installed synthesis pointer. Decoded output-side instructions.

The complete decoded core range `0x180007d10..0x18000cb30` yielded no computed-call instructions in the current listing; the retained scan result is native_core_output_dispatch.txt, an intentionally empty result. This is a bounded negative observation of decoded listing coverage, not proof that no reachable indirect call exists elsewhere or through a direct helper. No unsupported pointer chain was supplied.

The outer raw process has producer-selection branches and source/output pointer advancement according to its sample-format descriptor. The consumer instruction chain at `0x180132c6a..0x180132c8c` passes the input descriptor, module scratch and core into `0x180010040`, then copies the returned descriptor to the parent. Subsequent selected calls include `0x1800779d0` for an optional additional stage and `0x18011bd40` for the next output operation. Their full roles and the transfer to an active QMF synthesis dispatch remain unresolved. The room-path agent retains this consumer instruction evidence separately in `DolbyHeadphoneRoomResearch/native_path_consumer_instructions.txt`. Physical ear/channel assignment is still not established by the returned descriptor alone.

The size-64 transform selectors are now exact: `0x180088590(64)` selects `0x180106b60`; `0x180088170(64)` selects `0x180106ac0`. Their shared radix/helper chain was initially truncated by incorrect Ghidra no-return metadata for `0x180103500`. Decoded instructions prove a real `RET` at `0x1801035f9`; the selected wrappers pass16 to its counter-based loop. They continue after that call, perform three `0x180102f10` stages with indices1/2/3, and tail-jump to `0x180103730` with length64. There is no justification for stopping the native transform at the initial decompiler's no-return warning.

Only the private Ghidra database was repaired: `0x180103500` no-return cleared; the two wrapper body ranges extended over already decoded instruction bytes; and `CALL_RETURN` flow overrides at `0x180106ae2`/`0x180106b88` cleared. The exact before/after state appears in `ghidra_transform_repaired.log`, `ghidra_transform_body_repaired.log` and `ghidra_transform_flow_repaired.log`. Executable bytes were not edited. Regenerated complete wrapper bodies are in native_transform_repaired_functions.txt; complete radix helpers and tails are in native_transform_contract_functions.txt and native_transform_tail_functions.txt. The decoded continuation and tail jumps are retained in native_transform_contract_instructions.txt and native_transform_tail_instructions.txt.

The corrected wrappers identify the transform stages and argument flow but do not establish aggregate analysis/synthesis normalization or an active native PCM reconstruction path. No normalization constant was invented from transform names. Native hybrid recombination, core +0xb0 uptake, final physical ear routing and runtime gain/latency equivalence remain unresolved after this pass. No new model, nominal fixture, HRIR or SOFA was generated. The original consumer hash remains unchanged, and the bounded synthesis-chain search is stopped.
