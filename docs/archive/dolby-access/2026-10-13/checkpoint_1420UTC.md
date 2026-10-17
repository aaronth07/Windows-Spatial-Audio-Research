> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Checkpoint — 2026-10-13 14:20 UTC

## Current verified state

The 13:27 property-only candidate round trip and exact baseline restore remain verified. The later readback record confirms candidate and restored API/registry bytes, unchanged endpoint inventory, and Atmos-off before and after. Current playback is stereo 48 kHz PCM16; paired recording is stereo 48 kHz PCM24. The earlier PCM24 playback observation is historical. This operation proves property-store persistence and immediate restoration only.

The native-object helper, C# transaction wrapper, PowerShell runner, and capture comparator compile and pass offline tests. The preparation manifest now pins the corrected runner SHA-256 <private-artifact-digest>; the other four listed artifact hashes are unchanged. The runner configures 20-second native-process and 15-second BAT limits. Offline tests exercised child success/exit-1 propagation, termination/disposal with a shortened 150-ms deadline, and null callback rejection before COM activation; actual BAT behavior remains untested. No native runtime was released, no streams were opened, and no spatial BATs were run. Prior review of the latest five hashes and the six-capture baseline/candidate/restored plan remains pending. The desktop is clear for EQ; no reservation is in place.

Windows readback is Windows 11 Pro build 26200. Existing SoundVolumeView 2.36 compatibility is unverified. Runtime and capture behavior remain untested.

## Cache date-converter trace

The matching-PDB-verified converter at constructor 0x18274bc30 uses type 0x18021aff8 and callback 0x183661090. Its style 0x10 converts supported DateTime and DateTimeOffset values to UTC before formatting with InvariantCulture and pattern yyyy'-'MM'-'dd'T'HH':'mm':'ss.FFFFFFFK. The matching PDB symbol names are verified offline. This establishes the converter rule, not which PHRTF members use DateTime versus DateTimeOffset.

The exact cache contract remains incomplete: PHRTF property/type association and output member order, Guid serialization, and the complete activation/configuration schema are unresolved. Callback 0x18274be70 is general contract creation; it is not proven to filter PHRTF properties. The source capture UUID is a valid Guid, while decoded InstallationId is not directly parseable as a Guid and must not be substituted. No cache was generated or accepted.

Read-only registered Event Log inventory found no provider matching *Dolby*. This does not rule out private ETW or other diagnostics. No app/DLL execution, cache edit, stream, spatial command, or tracing setup occurred for this checkpoint.

## Evidence

Property readback: property_round_trip_verification_20261013.json. Native preparation and corrected five-hash manifest: native_load_preparation_20261013.json. Converter details: access_date_converter_20261013.json and reports/2026-10-13/cache_serialization_addendum_1408UTC.md. Schema limits: cache_schema_links_20261013.json. Event Log inventory: existing_dolby_log_inventory_20261013.json. Dot's release remains the next gate; the primary-authored native_load_validation_plan_1355UTC.md was not edited.
