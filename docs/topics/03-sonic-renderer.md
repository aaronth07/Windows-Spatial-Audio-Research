# Sonic coefficient paths and DSP replacement

The tested Sonic engine ran a custom SOFA DSP in an isolated process. System audio-host admission is a separate test.

## Legacy HRTF dataset versus active CPU DSP

The legacy dataset has a parser/configuration path and a serialized directional response layout. That fact is insufficient to show that the current headphone CPU implementation consumes it. In the inspected Flex2 factory, supplied legacy filter-bank interfaces are ignored at DSP creation and processing reaches embedded descriptors/tables. Replacing that file was therefore not a working coefficient update for the examined route.

Embedded tables are data in the implementation, not necessarily a room response. Distinguish direct directional coefficients, interpolation descriptors, delays, matrices, room filters and code. Identifying a float array does not establish its units or which branch uses it.

## Two actual factory routes

The native HrtfApo engine supports a factory override. A nonzero factory GUID reaches actual COM activation. A process-local manifest allows the unmodified engine to create an owned factory and custom DSP without changing the native engine's imports or vtable. Native control methods are forwarded with exact x64 tail thunks; the custom Render method can replace the samples.

The default zero-GUID path loads a CPU module by its expected basename. An owned module preloaded first in a fresh owned process can supply the required factory and data-helper exports. This is a loader-resolution result, not permission to replace a protected system file. The native and custom modules can coexist at distinct full paths. The native helper is loaded by an absolute System32 path and checked against a reviewed hash.

Diagnostic output first established the source-record ABI. A mistaken interpretation of the field at byte 112 as stride caused a retained failed probe; debugger evidence corrected it to frame count. Contiguous samples then matched the diagnostic reference. The selected source layout has position at byte 0, scalar gain at byte 12, PCM pointer at byte 104 and frame count at byte 112. Unmapped bytes are not assumed to hold another position or gain ramp.

## Full SOFA rendering

The custom engine prepares banks outside rendering and maps position to a nearest measured direction. A 128-frame partitioned kernel retains full 4096-tap filters. Input-time crossfades preserve earlier samples under their earlier filters, so changing position does not erase a prior response tail. An extended native test selected every one of 710 banks against independent convolution.

The current kernel supports 64 sources. The private native engine experiment uses fixed slots and does not prove how an actual system host's source pool is sized. A larger reserved pool, concurrent engines and active-source scheduling must be designed explicitly before deployment.

## Automatic preparation and safe startup

Loading a custom DLL was originally insufficient: its native helper pointer and dependency path were unset. The bootstrap now optionally calls InitializeCpuFactory before accepting the module. The custom hook reads an adjacent prepared SOFA container, validates dimensions, exact extent and payload digest, prepares filter state, and then publishes the native helper. Missing/malformed data fails initialization.

The helper entry point returns E_UNEXPECTED before readiness instead of jumping through null. Factory creation also fails safely while unprepared. A native module already resident under the expected basename produces ERROR_ALREADY_EXISTS, identifying a missed preload opportunity. Concurrent factory creation/release after readiness passed in an owned host. These checks do not prove that an actual audio host retries an early failure.

## Why system integration is separate

An inert APO has been built and tested as a possible early bootstrap. Static audio-host construction establishes a per-pipe opportunity before later Sonic creation, but not first-load order across all pipes. Basename selection is process-wide. An endpoint-registered effect does not prove that every engine in that process belongs to the endpoint.

Real deployment still needs admitted effect placement, exact endpoint-to-host identity/sharing, a per-engine scope or exclusive host, access to the module/data, source lifetime handling and removal of resident state during restoration. Restoring a property does not unload a module. No installed system-wide replacement is claimed.

The frontend's minimum-frame getter also runs before DSP factory selection. The tested private build rejects 128/256/480/512 frames before custom Render and accepts 1024/2048. A 128-frame custom kernel therefore does not make that frontend a 128-frame producer. A custom input frontend is a separate route.
