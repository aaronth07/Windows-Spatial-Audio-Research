> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Persisted fallback-control native test

Authorized by the user's permission to change selected virtual endpoint. Run only on its existing Dolby Atmos for Headphones selection, with the current personalized profile as baseline. No provider, default, format, routing, Runtime property, Access application, driver or service change is included.

Candidate `candidates/trailer_control_zero_v1/trailer_control_zero_api.bin` is 62,791 bytes, SHA256 `<private-artifact-digest>`. Change coefficient lane15689 from1 to0, and XOR checksum lane1 from1685 to1684. Only API bytes19 and62771 differ. Native size and fast-deserializer statuses are0, scratch canaries intact. The decoded fallback at PCA+0x15c is0; both directional fields and room taps retain the baseline decoded hashes. This is a diagnostic control, not yet a room-only toggle: an upstream geometry/gain branch also reads it.

Before any write, create new durable API, registry and full-context backups, save all six endpoint/context registry keys and selected/active formats, validate the exact candidate, and require baseline API SHA256 `<private-artifact-digest>`. Require all default render roles to remain selected virtual endpoint, Dolby Access closed, alternate DAFM absent, and current data identical to the fresh snapshot. Any mismatch or access denial stops the test.

Use the retained ten-position grid, including front/left repeats, one-metre radius, one native dynamic object, 48kHz float, SoundEffects category, -34dBFS one-second exponential sweep, 0.25-second lead and two-second tail. Record selected virtual endpoint through the paired virtual endpoint. Capture baseline twice, apply only normal Dolby context Init User PID1 `{1b4dab55-b1fb-4d8c-8317-f2d4a96efbb8},1` once with SetValue/Commit and exact readback, then capture candidate twice. Restore the fresh personalized bytes and capture restored twice. Each run checks provider and expected profile at both ends. A failure triggers the same exact restore in finally; no broader changes follow a failed test.

Compare direct spectra, ITD, ILD, early/late energy and repeatability without normalization or separate ear/direction alignment. Only one common stereo-field lag is permitted. A changed direct response prevents claiming independent room control. A null result does not establish rejection, because live overrides, source category and gate counter remain unobserved.

Restore API94b3…/registry04a3… from this test's fresh backup, preserving every other User property and all saved keys. Verify all six saved registry keys and selected/active Dolby formats after restoration. Preserve raw captures, all command logs, failures and restoration observations. Do not restore the old unpersonalized baseline.
