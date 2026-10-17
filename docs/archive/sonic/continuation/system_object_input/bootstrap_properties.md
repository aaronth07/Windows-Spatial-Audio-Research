> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Bootstrap initialization properties

The unregistered prototype reads only the following private keys from the supplied APOInitSystemEffects/2 `pAPOSystemEffectsProperties`. Their format ID is the prototype CLSID `{27E623F8-71CE-40D1-AE8C-BCAC46A71243}`. No registry writes are performed by the module or test.

| Property ID | Type | Meaning |
|---|---|---|
| 1 | VT_LPWSTR | Absolute path of a new startup event file; maximum 1023 characters |
| 2 | VT_LPWSTR | Optional absolute CPU-factory DLL path; maximum 1023 characters |
| 3 | VT_BOOL | Explicit preload enable; defaults false |
| 4 | VT_LPWSTR | Approved SHA-256 of the factory file, exactly 64 hexadecimal characters; required when preload is enabled |

Absent keys leave the object inert. If any bootstrap setting is supplied, a valid absolute new log path is required. Malformed values, relative paths, a missing approved digest and hash mismatch are rejected. Preload additionally requires a valid endpoint GUID supplied through standard PKEY_AudioEndpoint_GUID in `pAPOEndpointProperties`. Discovery-only initialization skips preloading. The file digest is checked before configuration and again before LoadLibraryEx; both required CPU exports must exist. This checks the disk file and does not establish mapped-memory integrity or Windows admission.

The positive property-store test never calls ConfigureBootstrap before Initialize. It uses an owned in-memory IPropertyStore, confirms the first logged Initialize event has no CPU module and the subsequent event has the pinned native CPU module with matching path and SHA-256. Logging-only initialization, discovery, absent settings, malformed types/digests, relative paths and mismatched digest are included in 13 property cases.

ConfigureBootstrap remains an owned-host convenience. Its explicit full-path caller establishes a disk-file hash baseline; the test wrapper independently pins that file before launching the host. System property configuration instead requires the supplied approved digest. The prototype maintains one configured process-level log/preload state. It is not a multiple-endpoint dispatcher.

On the initialization-property route, constructor logging is unavailable before those settings arrive; the first record is Initialize before preload. On the explicit Configure helper route, constructor logging is available. Event phases and the binary record structure are documented in bootstrap_api.h and bootstrap_test_plan.md. Logging and hashing occur outside APOProcess and stop after 64 records.

The native CPU DLL is used as the explicit preload target for this inert candidate test. Selecting the custom factory through a real endpoint remains a separate experiment, with host scope, first-load timing and restoration unresolved.
