> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Native room fallback control takes effect

Changing coefficient lane15689 from1 to0, with only its checksum repair, produced repeatable additional early/late output in the native Dolby object path. The exact personalized configuration was restored. This explains an active mechanism behind the current short-tail setting; it does not alone prove that every prior perceptual difference came from this field.

Session 20261014T2148321662643Z_trailer ran six complete captures: current twice, control0 twice, restored current twice, ten fixed positions each. Provider stayed Dolby; normal Init User PID1 was the only written property. Every saved endpoint/context registry key matched byte-for-byte afterward, selected/active formats stayed Dolby, and API94b3…/registry04a3… were restored. Exact plan, analysis.

| Comparison | Relative response L2 error |
| --- | ---: |
| Current repeat |0.00175127|
| Control0 repeat |0.00215510|
| Restored repeat |0.00176293|
| Current→restored |0.00270049|
| Current→control0 |0.0898090|

One common stereo-field lag per comparison removes capture transport differences. No gain normalization, separate ear alignment or separate direction alignment occurred. Within each run, all ten direct peak locations were consistent. Maximum recorded PCM magnitude was0.02463, below the guard; all positions completed.

Across ten directions and both ears, relative to current control1, control0 changed direct5ms energy by−0.0540..−0.0701dB, early5..20ms energy by+8.8535..+11.3406dB, and late20..250ms energy by+20.6992..+22.3916dB. The250..900ms window stayed near the recorded floor, with changes−0.0128..+0.0684dB. These windows describe output timing, not isolated named DSP stages. All peak ITD differences were0 samples; maximum direct-window ILD change was0.00114dB. Median absolute direct-spectrum changes were approximately0.0536..0.0700dB, consistent with the small attenuation rather than a new directional coefficient set.

The native parser observation shows unchanged directional PCA field and room-tap hashes, fallback+0x15c now0, successful size/deserializer status0 and intact bounded scratch canaries. The field has additional native room-send and gate consumers. Profile1 mapping confirms suppression of room sends and shared normalization; its current Windows source-index producer remains unobserved. Static branch semantics plus this isolated A/B/A support actual control consumption and room excitation. The small direct attenuation is retained and disclosed; unchanged direct amplitude is not claimed.

Next, hold fallback0 fixed and independently test room output taps and feedback. A room-only coefficient control must show a repeatable room change with direct transfer preserved against this same enabled reference. Arbitrary measured-room FIR replacement remains a separate representational limitation of the native recursive network.
