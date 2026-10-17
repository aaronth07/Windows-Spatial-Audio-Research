> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Enabled-room coefficient test

The prior fallback-control A/B/A completed with exact restoration and a repeatable room-tail increase. This follow-up keeps that control fixed at0 while changing one room coefficient group at a time. It is covered by the user's authorization to change selected virtual endpoint. Keep Dolby selected, all defaults/routing/formats unchanged, and Dolby Access closed.

Fresh backup: current personalized API62,791 bytes SHA256 `<private-artifact-digest>`, registry62,799 bytes SHA256 `<private-artifact-digest>`. Save/read back both plus complete Dolby User context, all six endpoint/context keys and selected/active formats before a write. Require all three default render roles to remain selected virtual endpoint, no alternate DAFM, pinned input/executable hashes and exact current state at each transition.

All profiles retain directional coefficients and shared exponent/distance controls. The enabled reference changes only fallback lane15689 and checksum from the current baseline. The two candidates below differ from that reference in only the named group plus checksum. All three pass the pinned native size validator and fast deserializer with statuses0, intact scratch canaries, unchanged directional hashes and the expected decoded room arrays.

| State | API SHA256 | Change relative to enabled reference |
| --- | --- | --- |
| Enabled reference | `<private-artifact-digest>` | Fallback0; original room coefficients |
| Half feedback | `<private-artifact-digest>` | Half-scale signed Q15 real/imaginary feedback lanes1050..1561, ties-to-even; API[4215,6263) and checksum[19,23) |
| Muted output taps | `<private-artifact-digest>` | Zero tap lanes1578..1833; API[6327,7351) and checksum[19,23) |

Half feedback retains delays, allpasses, matrix and output spectrum. The original matrix norm is1 and maximum complex feedback magnitude approximately0.78223; halving gives a sufficient contraction bound below0.392, including Q15 rounding. No delay/allocation changes occur. This is an alternative parametric decay, not an arbitrary measured room FIR or calibrated T60 setting.

Sequence: apply enabled reference and capture twice; change only feedback and capture twice; restore enabled reference and capture twice; change only output taps and capture twice; restore exact personalized baseline and capture twice. Each transition writes only Dolby normal context Init User PID1 with SetValue/Commit, verifies the full expected User state, then verifies all saved keys. Every capture checks selected provider/profile at both ends. Use the same ten positions,48kHz one-second ESS,-34dBFS,.25-second lead,two-second tail and paired selected virtual endpoint capture as the previous test.

On any failure, inspect the current profile hash and restore only from one of these explicitly reviewed states to the fresh API94… backup. Refuse any unrecognized external profile or unrelated context drift. Final verification requires baseline94/04, all six registry keys byte-identical, Dolby formats unchanged and restored captures retained. No provider toggle, Access launch, default/routing/device-EQ/driver/service change is included.

Compare enabled/feedback/enabled A/B/A and enabled/muted responses, including repeatability, direct spectra/ITD/ILD and early/late energy. Subtract matched outputs only after one common stereo-field lag; no per-ear/direction timing or level normalization. Preserve raw responses and the exact personalized baseline. Any direct change is reported, not normalized away.
