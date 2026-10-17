> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Cache serialization trace — 2026-10-13 14:08 UTC

The custom settings constructor `0x18274bc30`, reached by PHRTF serialization helper `0x1835c23f0`, installs a contract resolver and a date converter. Focused Ghidra traces establish constructor `0x18274bd90`, date-converter type `0x18021aff8`, and its write callback `0x183661090`. The previously labeled `function_contract_property_filter` (`0x18274be70`) is a general contract-creation callback; it does not by itself select PHRTF members or establish their order.

The constructor sets converter style to 0x10 and culture to the independently resolved InvariantCulture accessor. The write callback checks boxed DateTime and DateTimeOffset, applies UTC conversion when bit0x10 or0x40 is set, formats the result, then writes the resulting string. Matching SharedLibrary PDB names corroborate all six selected custom imports:

| Access slot | Framework target | Matching public symbol |
| --- | --- | --- |
| 181d79298 | 180bc9e20 | System::DateTime.ToUniversalTime |
| 181d79290 | 180bd86e0 | System::DateTime.ToString |
| 181d794c8 | 180bb97c0 | System::DateTimeOffset.ToUniversalTime |
| 181d794c0 | 180bc7490 | System::DateTimeOffset.ToString |
| 181d82b28 | 180011088 | Boxed_DateTime |
| 181d82b80 | 180011210 | Boxed_DateTimeOffset |

The default native string at `0x181eb90d0` is `yyyy'-'MM'-'dd'T'HH':'mm':'ss.FFFFFFFK`. With constructor style0x10, both supported date types are converted to UTC before invariant-culture formatting. This resolves the serializer's date-conversion rule. Which PHRTF JSON members are represented as those date types, exact member association/order, Guid text serialization, and complete phrtf.json activation/config schema remain unresolved. Source JSON member order still must not be substituted for native output order.

Evidence: `ghidra_json_settings_constructor_20261013.txt`, `ghidra_custom_contract_20261013.txt`, `access_custom_contract_types_20261013.json`, `access_date_converter_20261013.json`, `framework_pdb_date_symbols_20261013.txt`. PDB identity is the same verified GUID13c8dcdb-8cde-4ce1-bb43-877b7f8a4034 age1; 217,659 public-stream records yielded31 selected matches, including six new date targets. Symbols were read without application to Ghidra. The new resolver/date script ran and compiled; the optional extra-target argument in the PDB reader was fixed and the successful Ghidra run checked. No app/DLL execution, cache file edit, stream or spatial command occurred.

Further bounded string inspection locates compiled metadata names at file offsets around21,303,000: PersonalizedHrtf; PhrtfCaptureMetadata; VirtualizerParameters; MetadataVersion, Uuid, UserName, CaptureContext, CaptureSubmissionDate, PhrtfCreationDate, SuezVersion, CaptureType; Coefficients and RoomModel. Config metadata names include PhrtfState, SettingsFileName, ServerResponse, UserId, OktaUserId and UserProfile. Separate attribute-name strings around23,313,680 include phrtf_active, settings_file, server_response, personalized_hrtf, phrtf_capture_metadata, virtualizer_parameters, capture_submission_date and phrtf_creation_date. Exact name searches are recorded in `access_selected_model_name_search_20261013.json`. Physical string order is not serialization order, and adjacent strings do not establish a field-to-offset or JsonProperty association. The stored digest at config+0x18 must not be relabeled as a server/account field based on adjacency alone. The next static link is decoding those compiled metadata member/attribute handles, rather than analyzing the general contract factory again.
