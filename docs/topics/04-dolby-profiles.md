# Dolby profile storage and native loading

## Consumer Access versus other products

Consumer Dolby Access/Atmos for Headphones, professional Dolby Atmos Renderer, and Windows' native spatial frontend are distinct targets. A schema or parser from one does not establish the other's import contract. The research keeps consumer storage, endpoint property delivery, coefficient decoding and SOFA recovery as separate evidence tracks.

## Consumer storage trace

Pinned consumer-binary analysis found PHRTF filename patterns, storage/configuration types, async read/deserialization, typed casts, save/current-setting paths and a settings integrity check. The examined check serializes settings, calculates an uppercase MD5 hex digest and compares it with a configuration field.

That is payload integrity evidence, not an entitlement/account bypass. File discovery, a passing checksum or a UI flag does not prove the native renderer loaded the coefficients. Earlier storage traces left exact schema/root/indirect-consumer questions open; those archived findings must not be rewritten as a verified consumer UI/cache import.

## Endpoint property delivery

A later route uses the endpoint's native effects property store. The Dolby effects-context identifier is `{45da5c30-2837-4ac4-b1e2-50acc3865974}`; the headphone initialization property is `{1b4dab55-b1fb-4d8c-8317-f2d4a96efbb8},1`. These are shared software identifiers, not device instances. A live endpoint is resolved separately.

The observed API and persisted registry representations differ by an eight-byte envelope. A typed initialization payload carries optional controls and versioned coefficient data. The tested 1.1.0 profile has 15,691 signed Int32 entries. Native decoding narrows their low 16 bits before rp deserialization. The public wire example performs structural inspection only and never writes an endpoint.

Native object-rendered comparisons establish candidate-dependent output. Exact transaction readback separately establishes the bytes accepted/persisted by the property path. Neither claim alone establishes anatomical correctness, a supported vendor import feature or permanent persistence through future device/vendor changes.

## Structural checks

The inspector checks framing, optional-value flags, bounded array/version lengths, integer layout, trailing bytes and the low-word XOR checksum seeded with `0xA569`. The checksum is repaired when an offline field edit changes covered lanes. Its role is different from the consumer settings MD5 and from SHA-256 used to identify artifacts.

The v1/v2/v3 representation precedence matters: an apparently correct newer field can be ignored if another representation shadows it. Preserve unrelated initialization fields and establish which representation the native consumer selects. Do not infer the active branch from the file extension.

## Delivery evidence requirements

A complete mutation experiment needs the exact immediate current baseline, not an older historical default. API readback, persisted representation, unrelated context properties, provider/default/format state, cleanup and repeatability are independent checks. Readback success proves state access; a fresh native rendering comparison provides processing evidence. A profile hash appearing in logs is not coefficient-use evidence by itself.

The public repository supplies no automatic endpoint writer or personal coefficients. It documents the property contract and offers offline examples so readers can examine their own data without conflating parsing with native use. Archived application reports describe their historical tests and remaining compatibility limits.
