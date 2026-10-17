> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Dolby room output-tap test

Authority: the user's October14 approval permits changes on selected virtual endpoint. This test implements that authorization with fresh backups and exact restoration. The current personalized profile remains the restoration target.

Only the normal Dolby User property `{1b4dab55-b1fb-4d8c-8317-f2d4a96efbb8},1` in context `{45da5c30-2837-4ac4-b1e2-50acc3865974}` on selected virtual endpoint is set through `IAudioSystemEffectsPropertyStore` and `IPropertyStore.SetValue/Commit`.

The 62,791-byte candidate zeroes coefficient lanes1578–1833, the four64-tap room output blocks, and repairs checksum lane1. Its allowed API byte ranges are `[19,23)` and `[6327,7351)`. Directional PCA, exponent, room delays, feedback filters, ear filters, mixing matrix, distance trailer and initialization controls retain their exact bytes. Runtime properties retain their exact bytes.

| Payload | SHA-256 |
|---|---|
| Current personalized API / restore target | `<private-artifact-digest>` |
| Current registry representation | `<private-artifact-digest>` |
| Room output-tap candidate API | `<private-artifact-digest>` |
| Candidate registry representation | `<private-artifact-digest>` |

Sequence: verify Dolby is already selected and active, then save and immediately verify the API/registry BLOBs, all six Dolby User values, both endpoints' configuration and Dolby/DTS Default/User stores in a new experiment directory. Check that the alternate DAFM initialization is absent. Two baseline captures are followed by the isolated candidate write and two candidate captures. Restore the exact saved personalized profile before the two final captures. An outer cleanup repeats idempotent restoration on any failure and verifies the saved configuration and selected Dolby format.

Each capture starts a fresh `ISpatialAudioClient` dynamic-object stream and records the paired selected virtual endpoint virtual endpoint:48kHz, ten fixed directions, one-second20Hz–20kHz exponential sweep at−34dBFS,250ms pre-roll and two-second tail. The six captures take approximately198seconds. All prepared input/executable hashes are checked. Only the process owned by the test is stopped on timeout or capture failure. Maximum capture duration is60seconds per run. Invalid state, access denial, missing/unreadable backup, wrong provider/profile, clipping, discontinuity or unrelated property changes stop the test and enter restoration.

No provider selection, default endpoint, routing, format, Access GUI, driver, service, license, device EQ or other Dolby property change is included. The old14-byte unpersonalized initialization is excluded from the controller. Windows' evidenced spatial notification sequence, if changed by the property operation, is recorded separately and never rewound.

Analysis preserves stereo timing and relative levels. Deconvolution uses the retained inverse sweep and QPC transport timing. One common whole-field lag may compensate a capture transport offset. Repeated baselines, candidate repeats and restored captures define repeatability. Compare the direct peak/early response and subsequent tail separately; a short baseline tail or an unchanged candidate alone does not prove native room-pointer selection.

The controller compiled; eight in-memory transaction cases pass, including partial failure and idempotent restoration. Fifteen offline payload/room/runtime test groups pass. Fresh native inspection confirms Dolby and the expected personalized hash before this test. These checks precede native delivery; they are not a claim that the candidate was accepted by the renderer.
