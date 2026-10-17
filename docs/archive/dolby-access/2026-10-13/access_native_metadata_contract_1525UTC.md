> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Access PHRTF metadata contract

The copied Access DLL contains a NativeFormat metadata root at file offset 18,622,177. Microsoft CoreRT's reader schema decodes 147 scopes; the header ends at offset 300 and its first scope begins there. Selected scope/type/property/attribute records form consistent references. The bounded local reader checks the Access SHA-256, reads only the containing image section, rejects malformed integers and out-of-range records, and never executes the downloaded reference sources or Dolby code.

`scripts/inspect_access_native_metadata.py` passed integer width/sign boundaries, malformed/truncated input, UTF-8 string/collection, unnamed root namespace and typed attribute-constructor association checks. Initial inspection exposed an unnamed-root handling error and an incorrect generic interpretation of `QualifiedMethod.EnclosingType`; both were corrected and covered by the synthetic record test. Output is `access_native_metadata_models_20261013.json`, including metadata offsets and public reference source hashes.

Verified configuration associations:

| Managed property | Type | JSON name | Attribute |
| --- | --- | --- | --- |
| PhrtfState | PhrtfActivationState | phrtf_active | Required; custom activation-state converter |
| SettingsFileName | String | settings_file | Required |
| Checksum | String | correlation_id | Required |
| ServerResponse | PhrtfServerResponse | server_response | Optional |
| UserId | String | user_id | Optional |
| OktaUserId | String | okta_user_id | Optional |
| UserProfile | String | user_profile | Optional |

The activation enum has Inactive=0 and Active=1. The custom converter's actual JSON token representation is still unresolved. The server-response enum has None=0, Success=1 and error/status values 2–9; a local import must not fabricate a server/account success.

Settings map `PersonalizedHrtf` to `personalized_hrtf`, then `PhrtfCaptureMetadata` to `phrtf_capture_metadata` and `VirtualizerParameters` to `virtualizer_parameters`. Metadata `Uuid` is Guid with Dolby.Common.Json.GuidConverter; both date properties are DateTimeOffset with property-specific IsoDateTimeConverter attributes. Version properties use System.Version/VersionConverter. Virtualizer Coefficients is Int32[] mapped to rosella_coefficients; coefficient version and room_model are required.

The two property-specific ISO converters are material new evidence. The previously traced global converter uses style 0x10 and UTC formatting, but that alone does not establish the effective formatting of these attributed date properties. Converter precedence, the custom Guid output, final JSON member ordering and activation token encoding must be resolved before computing an accepted cache checksum. The observed Checksum-to-correlation_id association is a direct metadata relationship, not an adjacency guess.

Metadata property order is recorded as metadata order only. No cache was generated, accepted, installed or written to Access, and no native/internal load result follows from these static contracts.

Reference: Microsoft NativeFormat reader schema (historical reference unavailable)
