> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Consumer native room selection

The static caller chain now reaches consumer `CDolbyAudioProcessingModule::EncodeAudioData`. Two native selection paths can retain the initialized room/PCA pair or substitute a zero-initialized built-in room structure while retaining the same PCA. Which branch the current Windows spatial-object stream actually takes remains unobserved; neither the measured shorter tail nor the stored personalized payload establishes that runtime choice.

Target: inert `DolbyAudioProcessing.dll`, SHA-256 `0fada40251880d03c657a357745d362799c93fb6024f733f1c8bf1057f5e4715`. Ghidra12.1.4 used the independent `ghidra_projects/RoomPath` project. No shared REA switch, native DLL/audio execution, settings write, playback, or public-model change occurred.

## Caller chain and consumer boundary

| Address | Resolved edge or predicate | Evidence |
|---|---|---|
| 0x180131490 | Consumer `EncodeAudioData`, identified by diagnostic. Initial guard requires parent+0x2e1 nonzero, parent+0x538 module pointer nonnull and module+0x68 initialized. Parent+0x98 is tested against7 before a computed jump table; its enum meaning is not assigned here. | Consumer instructions |
| 0x180132b7a..0x180132c7d | Inline `CDapVRModule::Process` branch loads module from parent+0x538; module+0x68 must be nonzero. Loads core from module+0x60, scratch from module+0x58, input descriptor from parent+0xb0; CALL at0x180132c7d targets0x180010040. Result descriptor copied to parent+0x70/+0x80. | Same complete selected exception-range instruction listing |
| 0x180010040 | For core+0xc0 nonzero, invokes0x18002d690; otherwise invokes0x18002e940. Both reach core0x180007d10. Its secondary fragments0x180010644/0x180010356 contain continuation loops, not separate public APIs. | Boundary functions, caller functions |
| 0x18002d690 | Invokes0x18002c2b0 when core+0xc0==1,0x18002cf70 when==2, then invokes0x180007d10. The0x18002cf70 branch was not decompiled in this bounded pass. | Selected functions |
| 0x180007d10 /0x18002c2b0 | Both call direct/room wrapper0x180025a80 and room configuration extractor0x180006750. Exact direct call-site addresses are retained in the file header. | Same |

Initial Ghidra decompilation omitted the consumer Process path because it did not resolve the jump table at0x180131524/0x18013152f. A raw read-only RIP-relative/call scan located the diagnostic reference0x180132b9e and CALL0x180132c7d. The PE exception directory bounds the enclosing function at0x180131490..0x1801331e8. The table at0x1801331a8 contains RVAs: indices0/1/3→0x180131531, indices2/4/5/6→0x180131539. Valid table targets were disassembled in the private project, recovering the instruction path. The symbolic decompile still excludes that path; assembly is the evidence for it. No missing symbolic parameters are invented.

This connects the consumer encoding module to room rendering, but does not independently prove which WinRT interface/caller, object-vs-bed descriptor contract, category or current endpoint state supplies this particular route. The consumer entry has data/vtable references at0x1803e5370/0x1803e54d0/0x1803e5548. Those interface edges and live enum values remain unresolved.

## Exact room pointer selection

Use `C` for core pointer, `m=*(uint*)(C+0x11c)`, `n=param_4[2]` inside0x180007d10, and `dso=*(int*)(C+0x28)`. The n field participates in channel-count processing; a Windows object-type interpretation is not assumed.

At0x180007d10 entry, selection flag `local_838[0]` is true exactly when `n>=2`, `m` is one of8/9/12/13, and `*(pointer*)(C+0xd0)==NULL`. Separate flag `local_588` is integer-bit1 (displayed by Ghidra as float1.4013e-45) exactly when `n>=2`, m is12/13 and dso==0. It is a Boolean-like bit flag here, not an acoustic scalar.

When the first flag is false, C+0xafc and C+0xb04 are cleared and this particular wrapper block is skipped. When true:

| Predicate | Room/PCA pair written through C+0x1810 | Transition state |
|---|---|---|
| local_588==0 | pair[0]=C+0x18 initialized room pointer; pair[1]=C+0x20 PCA pointer | Recreate wrapper atC+0x1808 ifC+0xafc==0; set afc=1,b04=0 |
| local_588!=0 | pair[0]=0x180014b10()→0x1802f6140; pair[1]=C+0x20 unchanged | Recreate ifb04==0; set b04=1,afc=0; reset0x1800162c0 state atC+0x17f8 |

Both transitions call0x180006750 on the initialized C+0x18 structure for allocation/configuration before choosing the actual pair. The presence of that extractor call therefore does not by itself prove the initialized room is consumed.0x180014b00 writes the pair pointer at wrapper+8 and sets wrapper+0x28 reset flag. In the substituted branch, the additional wrapper argument ordinarily pointing to C+0xd8 is setNULL.

The separate0x18002c2b0 path selects initialized room/PCA when `(m & ~1)==8`, i.e. m8/9, using transition flagsC+0xb00/b08. Otherwise it substitutes0x1802f6140 with the same PCA. Here the controlling predicate is output mode, without the0x180007d10 local_588 test. This metadata-processing path must not be equated with all Windows dynamic objects before its descriptor producer is resolved.

Mode15 with dso!=0 enters another core branch (`local_600`), initializes the original room/PCA pair using flagC+0xb0c, and touches the DSO stateC+0x36b8. That branch is additional processing; dso==0 does not generally mean primary room off.

## Initialized profile, default resource and empty structure

Consumer0x180122950 (`ConfigureDapVRInitParams`) zeros init+0x18/+0x20, calls deserializer0x180150220 and coefficient selector0x18011c450, then stores its returned room atinit+0x18 and PCA atinit+0x20. It copies the optional integer atdecoded+8 into init+0xc, and optional finalBoolean decoded+0x50055 into init+0x28. Core initializer0x180004d40 copies init+0x18/+0x20 into C+0x18/+0x20. The existing serialized current payload has integer0 and finalBooleanfalse; this identifies payload fields, not a live core readback.

Initializer allocates the primary room wrapper only if init mode0 and both room/PCA pointers are nonnull. Modes1/2 follow other initialization branches. The output-mode setter0x180003da0 is a separate control: it stores requested modeC+0x118, and0x180003590 transfers it to current C+0x11c. Initial C+0x11c is1. Do not confuse serialized init integer0 with current output mode0.

Setter permits output modes8/9/12/13 except that init mode2 coerces them to1. Init mode2 also remaps requested10→3,11→6,15→1. This confirms the selectors' enum values can exist under init mode0, but does not establish the endpoint's requested mode. Runtime application0x18011cd60 calls the setter; its actual current input was not read.

`0x1802f6140` lies in `.rdata`, section RVA0x190000, raw size0x280c00, virtual size0x280ad2, characteristics0x40000040 (initialized readable data, no write flag). Its first128 bytes at raw file offset0x2f5140 are allzero. Private Ghidra also reports `.rdata write=false`. The only discovered direct reference is getter0x180014b10. This is file/static-memory evidence; indirect/computed writes, a protection change, relocation-like fixups or a different runtime image have not been ruled out by the xref scan. No runtime contents were inspected.

This empty structure is distinct from default headphone tuning. Consumer0x18012e640 loads resourceID254/type255, calls0x180123ab0 to retain default decoded room/PCA at module+0x30/+0x40, and initializes the DAP-VR module.0x180124f50 first attempts configured decoded pointers, falling back to those defaults if either isNULL. Full functions are saved in initialization decode and module boundary. The exact configured selector0x18011c450 is not included in this new pass; its retained prior evidence establishes decoded coefficient routing, but current selection still requires observation.

## Enable and distance controls

Wrapper0x180025a80 initializes wrapper+4 (`param_1[1]`) from selected room+0x7c !=0 on reset. It calls0x180018b70 with direct state, PCA and that flag. If result iszero, direct output is copied without room-wrapper0x18001b700; nonzero result enters room processing and adds its return. A zero selected static structure therefore initializes this flag tozero in the file-derived case.

0x180018b70 reads direct-state+0x6c; if negative uses PCA+0x15c. With direct-state+0x14 nonzero and+0x18 zero, it advances counter+0x1c while below PCA+0xc; after the threshold it ANDs enable with `(effective_control==0)`. If+0x18 is nonzero it resets the counter. These are exact native predicates; their enum/control meanings and live values remain unresolved. The wrapper calls geometry0x18003e550 and direct/room-send0x1800420d0 before this return is added. Direction fields/phase details remain in the coefficient agent's retained native functions; no public-math correction was made here.

The previously verified default controls[103,32568,6698,5090,1,1,1,1] remain consistent. In core mode15 branch, C+0xab4 is shiftedleft6 for steerer distance, far/mid/near current scalesC+0xabc/ac4/acc are shiftedright1, near-anechoicC+0xad4 is sent to0x18001b4d0 and geometry config to0x180018ad0. Grouped modesC+0xadc/ae4/aec populate different source-index records. These fixed controls are not proved equivalent to object SetPosition metres or AudioCategory_SoundEffects. Other room-send geometry can also depend on metadata; exact current native equation/category mapping remains unresolved.

All selected functions are retained without excerpt truncation in the linked text files; the consumer boundary also has recovered selected assembly. Java scripts compiled/executed successfully in headless Ghidra, all runs completed and saved only the private project. A second pass checked init-vs-output mode, pointer offsets, branch Boolean representation and `.rdata` vs`.bss`; the gaps above remain explicit.

Needed runtime facts for a later authorized experiment: current C+0xc/+0x11c/+0x28/+0xd0, input descriptor n, selected pair[0]/pair[1], room+0x7c, wrapper enable and0x180018b70 result, configured-vs-default pointer identity, and actual source-profile/category record. A reviewed isolated tap comparison can establish an output effect without mutating hidden fields, but unchanged output alone cannot distinguish ignored tuning, empty/alternative room selection, or zero room send. No current mode is inferred from the captured tail.
