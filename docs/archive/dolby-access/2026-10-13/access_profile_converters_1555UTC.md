> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# PHRTF profile converter findings

The cached configuration's `phrtf_active` value is a JSON Boolean. Native writer `0x18367e080` calls the JSON writer's Boolean overload with `state == 1`; metadata defines Active=1 and Inactive=0. It is separate from the settings-hive string `HasSyncedProfile`.

The metadata UUID converter writes through `System.Guid.ToString` with format `D`. The Access writer is `0x1836cc440`, its custom framework import slot is `0x181d79b68`, ordinal 10720, and the copied framework target is `0x180be2c70`. The exact matching PDB names this target `System::Guid.ToString`. Ghidra shows it passing the native format string at `0x18057f9e0`; the measured UTF-16 string is `D`. The profile filename separately uses the boxed Guid with `phrtf_{0}.json`.

Converter bindings use the CoreRT name-hash algorithm, independently matched against the previously traced PhrtfSettings, PhrtfConfiguration and IsoDateTimeConverter type records. Each new converter hash occurs exactly once with consistent class header/parent/function pointers. GuidConverter is `0x180158d38`, activation converter `0x180158550`. The generic wrappers invoke typed writer/read callbacks at physical offsets +0x60/+0x68. The leading table word at +0x18 is not a callable slot; the earlier raw table listing must not be interpreted as virtual-method indices. The two shared flag thunks both jump to a `mov al,1; ret` body.

Both typed readers preserve the supplied existing value when their existing-value flag is true. Otherwise Guid reads the JSON value as String and invokes the Guid parser/constructor; activation reads a boxed Boolean and returns enum 0 or 1. Whether a fresh cached-model parse supplies an existing value to either converter remains a call-site question. Do not assume that the input UUID/activation token becomes the final cached object solely from the writer contract.

Property-specific ISO converters on the two DateTimeOffset properties still require precedence/default-constructor/read-path verification. Final serialized member order and the runtime object's exact canonical data remain necessary for the MD5/ASCII cache check. No accepted cache sample exists, and no Access cache/hive was edited or app launched.

Reproducible bindings are in `scripts/resolve_access_profile_converters.py` and `access_profile_converters_20261013.json`. Actual traces are `ghidra_profile_converters_20261013.txt`, `framework_pdb_guid_symbols_20261013.txt`, and `ghidra_framework_guid_20261013.txt`. The scripts compiled and executed offline; selected Ghidra runs completed and saved, PDB GUID/age matched, and three type-name hash anchors passed. This is shipped-code evidence, not runtime cache acceptance.
