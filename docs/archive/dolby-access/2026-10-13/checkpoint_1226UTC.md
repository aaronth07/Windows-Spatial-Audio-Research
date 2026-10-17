> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Dolby Access PHRTF checkpoint — 2026-10-13 12:26:33 UTC

This checkpoint records the newly verified outer digest-byte encoding and current static operation. It is a dated update to the 12:17 evidence snapshot. Work remains offline static analysis plus read-only getter observations; no live cache file or setting was changed.

## Verified digest encoding

- `access_digest_encoding_verified_20261013.json` resolves the consumer Access settings digest helper at VA `0x18367ddd0` to MD5 with uppercase hexadecimal formatting (`X2` per digest byte) and US-ASCII byte conversion using code page 20127.
- Access custom module slot `0x181d7e5b8` is not an ordinary selected PE IAT import. Its application module header descriptor at `+0x108/+0x10c` is validated against the actual framework PE import; the custom imported array starts at `0x181d77008` and contains 8,142 entries. The .NET Native framework root `0x1801a2b20` resolves custom ordinal index 25702 to accessor `0x180b8baf0`.
- Static accessor/constructor chain: `0x180b8baf0` -> cctor record `0x180566918` -> constructor `0x18066c510` -> encoding type `0x18005e0e0` via `0x18088bc30` -> base constructor `0x18088ac80` with `0x4e9f` (decimal 20127). The conversion vtable entry is at offset `0xf0`, target `0x1807fb360`.
- Evidence records and traces: `access_digest_encoding_verified_20261013.json`, `access_framework_imports_20261013.json`, `ghidra_framework_accessors_20261013.txt`, `ghidra_framework_static_ctors_20261013.txt`, `ghidra_framework_encoding_ctor_20261013.txt`, and `ghidra_framework_digest_encoding_20261013.txt`. Resolver: `scripts/resolve_access_framework_imports.py`.
- The first resolver hypothesis treated the slot as a conventional PE IAT import and did not find the selected targets. The corrected implementation validates the custom descriptor/import mapping and bounds; rerun and `py_compile` passed. Focused Ghidra import analysis was `noanalysis`. No DLL was executed. The `dotNIET_plugin.py` source was used only as a code-format reference; it was not installed or run. Microsoft documents `ASCIIEncoding` as code page 20127 and describes default constructor behavior, but the target's fallback configuration remains unverified.

## Limits and current operation

Exact JSON member selection/order/indentation, concrete fallback behavior for this target, and accepted cached-profile state remain unresolved. The result does not establish that a cached profile is accepted or personalized output is active.

The primary agent's current operation is a bounded trace of concrete encoding fallback and JSON formatter behavior. The exact-property proposal remains pending direct <identity-removed> approval; no test setter, Commit, registry write/import, app-state change, or audio probe occurred. No rollback was needed. Any approved live test still requires fresh independently verified backups. The later audio probe remains separate and is not execution-ready.

## Source references

The custom .NET Native module/import format was compared against the `dotNIET_plugin.py` source (historical reference unavailable)`ASCIIEncoding` documentation](http<local-path>

## Exact paths

- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
- `<local-path>`
