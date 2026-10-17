> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Proposed native profile persistence test

The completed test establishes reversible native processing changes while the owned PHRTF tuning is stored in the consumer Dolby headphone User property. The shortest installation path is this property plus the existing Dolby Atmos for Headphones spatial selection. Access cache/UI integration remains a separate path. The existing six captures are being characterized offline before describing the change as a listener-specific HRTF effect.

The next proposed test checks whether the same profile survives an Atmos OFF/ON cycle. It has not been run and this document grants no release. A concrete runner and its hashes will be provided for Prior review before execution.

The only persisted profile change is `{1b4dab55-b1fb-4d8c-8317-f2d4a96efbb8},1` in context `{45da5c30-2837-4ac4-b1e2-50acc3865974}` for selected virtual endpoint playback endpoint `{<identifier-removed>}`. Its candidate API payload remains 62,791 bytes, SHA-256 `<private-artifact-digest>`. The active/default spatial selection changes between OFF and Dolby `{<identifier-removed>}` using the same two hash-checked user BATs, with fresh console/multimedia default guards immediately before each invocation.

Access remains closed. No Access app file, LocalCache JSON, settings hive, registry reference import, account, license, routing, endpoint format, driver, service, security setting or other User property is intentionally changed. Fresh backups include the exact baseline API14/REG22 value, all six User context values, scoped context export, Access settings/cache files and their absence inventories. They must be verified readable before the first mutation.

Proposed sequence:

1. Require the approved defaults/formats, closed Access, original property, DAFM VT_EMPTY and Atmos OFF. Enable Dolby and capture one fresh baseline with the already reviewed native object executable.
2. Set/commit only the unchanged candidate through the reviewed property API transaction. Verify exact API and registry readback and unchanged other User values. Capture `candidate_before_toggle`.
3. Stop owned streams, disable Atmos, and read the fixed User property again. It must still equal the candidate. Enable Dolby again and require candidate API/registry bytes, normal branch state, approved defaults/formats and unchanged Access files. Do not rewrite the candidate after a mismatch.
4. Capture `candidate_after_toggle`, then restore the exact fresh baseline inside the C# transaction's inner finally. Verify all six User values and API/registry hashes before capturing the restored baseline. Disable Atmos in the outer finally and verify OFF plus baseline/context/app-file/default/format restoration.

Four ten-second runs contain twenty impulse samples total at peak 0.01. Native process limits remain 20 seconds, BAT limits 15 seconds, exact one dynamic object and mono float32 48 kHz input, paired stereo 48 kHz capture, no fallback endpoint/format. Reserve one exclusive window, at most three minutes including cleanup. Any failure ends the sequence; fixed-property restoration precedes default/OFF checks. A blocked OFF guard records incomplete cleanup and exact current state rather than targeting another endpoint.

The offline comparison must preserve common stereo timing/levels and require candidate-before/after agreement beyond baseline/restored variation. Success would establish this spatial-toggle lifecycle only. Reboot, Access launch, profile/profile-mode changes and user-facing cache acceptance would remain untested. Access launch is deferred until its expected writes and restoration procedure are bounded independently; clean hive structure alone is insufficient grounds to replace a live hive.

If this lifecycle passes, recommend a separately reviewed native installer that backs up this one property, commits the verified profile, selects Dolby on the approved endpoint, and provides exact property/OFF restoration. Leaving the profile installed is a distinct final-state approval; the proposed test restores the baseline and OFF. For Access-managed import, finish the configuration/converter/serialization contract and prepare an offline cache candidate before proposing any cache writes.

Evidence: the actual completed manifest/comparison in `private/native_load_validation/transaction_20261013T150234101Z`, `native_load_verification_20261013.json`, and the unchanged reviewed C++/C#/PowerShell sources. Raw WAVs and per-stage logs are preserved. No further live sequence is released.
