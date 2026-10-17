> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# PHRTF room suppression: current findings

Rechecked 2026-10-15. No live audio, payload write, provider change, Access launch, or endpoint configuration change was performed in this pass.

The personalized file did not merely supply directional HRTF coefficients. Its serialized coefficient block also supplied room coefficients and a persisted suppression control. The owned file, initial API payload and registry representation agree exactly on coefficient lane 15689 = 1. Native decoding places that signed low-16-bit value at PCA+0x15c. Import preserved the source value; this was not a newly invented import flag or evidence of missing room coefficients.

The field can suppress source-profile-1 room sends and later disable an otherwise enabled room processor. Setting it to 0 permits the room return in the measured native object configuration. The exact isolated API changes are byte 62771 (1 to 0) and checksum byte 19 (149 to 148); both 62791-byte payloads remain the same length. Candidate generator tests verify that no other byte changes. This is specific to the pinned payload/layout and consumer, not a universal offset patch for arbitrary personalized_headphone files.

The earlier native A/B/A changed only that field and checksum. It increased 20–250 ms energy by 20.70–22.39 dB, and changed direct 5 ms energy by about -0.05 to -0.07 dB. It preserved personalized directional coefficient bytes and peak ITD, but the slight direct-level change means this flag is not a perfectly independent wet-only control.

For repeatable room-only comparison, the existing On and Off states both hold that flag at 0. Off zeros the mapped 256 room-output tap lanes with checksum repair; On retains them. Their measured direct-response differences are within reference repeatability, while Off reduces 20–250 ms energy by 20.75–22.45 dB. HalfFeedback retains the room and halves the complex feedback coefficients, producing an estimated T60 from a T20 fit of about 126 ms versus 226 ms. Those are renderer diagnostics rather than physical-room calibration. Restore restores the exact original low-reverb personalized payload, rather than the unpersonalized profile.

## Rechecked evidence

All ten original room-group raw WAVs were deconvolved again with the original compatible NumPy 1.24.4/SciPy 1.10.1 stack. Each float32 response matched its retained array exactly. All enabled-to-feedback, enabled-to-muted and enabled-to-restored response comparisons and per-window metrics reproduced the saved JSON values. Raw-file SHA-256 values and comparison results are in 2026-10-15_raw_capture_recheck.json. No raw capture or previous analysis was overwritten.

The first exact-parity check used the workspace's newer NumPy 2.4.6/SciPy 1.18.1 stack and detected differences of approximately 2.9e-8 maximum absolute amplitude, relative L2 2.4e-8, with zero lag. Repeating with the original dependency versions restored exact equality. This was numerical-library drift, not a newly measured audio change. The retained measurements were not altered to make the check pass.

Thirty-four offline room tests passed, and eight in-memory transaction tests passed. The current read-only Dolby API getter returned the exact original personalized SHA-256 <private-artifact-digest>. Separately, active-provider inspection returned DTS GUID {<identifier-removed>} on selected virtual endpoint. Thus the stored Dolby profile is intact, but Dolby rendering is not currently selected. No new Dolby playback verification is claimed.

## New source-profile producer trace

The consumer binary identity remains SHA-256 0fada40251880d03c657a357745d362799c93fb6024f733f1c8bf1057f5e4715. REA/Ghidra re-established the following chain:

1. 0x18002d690 calls queued-metadata dispatcher 0x180013f70 with destination *(core+0x6e8), then dispatches object processing 0x18002c2b0 when core+0xc0 == 1. The field is an internal branch value; no public enum name is assigned.
2. Dispatcher 0x180013f70 selects retained timestamped metadata records and calls 0x18001d120 with the destination source-record array. The source records have stride 60 bytes (0x3c).
3. 0x18001d120 obtains optional profile metadata via 0x180045bb0. That accessor returns its pointer at metadata-object+0x70 only when metadata-object+0xa4 is nonzero.
4. Without that optional metadata, the parser writes profile 3 to every source-record+0x2c and writes 1 to source-record+0x28. Assembly confirms the scalar write at 0x18001d90c and paired writes at 0x18001d8d0/0x18001d8da. These are source-record fields, not the later descriptor fields with coincident offsets.
5. With optional metadata, 0x180045b40 extracts a two-bit value per source per selected subblock from a two-byte source slot. The first byte holds subblocks 0..3; the second holds 4..7. Values 0..3 map to the same profile index written to source-record+0x2c. Assembly confirms the selected write at 0x18001d7ed.
6. Existing evidence shows 0x18002c2b0 forwards source-record+0x2c to 0x18004b8c0, which places it in descriptor+0x28. Geometry then selects PCA+0x10+index*0x30 and applies the special suppression branches for index 1.

This closes the immediate producer gap inside the native metadata parser. It does not identify a Windows public API field, prove the active stream contained optional profile metadata, assign a public name to index 1, or observe that historical stream's internal value. The raw A/B/A remains the runtime causal evidence under the tested conditions. Trace evidence IDs and full dossiers are retained in the adjacent 2026-10-15_*REA.json files:

- consumer: ev_a6d719eaea061b3ddc12f9ca6067d95d5b38a0348df4e0b69745842b72d21275
- caller: ev_7fe32fedfc0b9b9632770288a5c6bfe4a9b3734fe9e0133f7305882f05fe0f24
- dispatcher: ev_8c6a29c59b615e9dbd70faf511413e4ddf0001572eac6d9134d958b694fdd62b
- metadata writer: ev_069f03a6b1c7d1b110929dc8b2a120b0449e5bdb6a18d5b701aa1939438ed14c
- packed profile reader: ev_2ef3ff0c113f2e6369e8de4932e90e9a55df94ec08b5f4ccb6f18093fb02e863
- optional metadata accessor: ev_0fa3be80ff98f3913f8ef8eb4d7acd28d0d8ba6e2ffa8b03391f2be0a8811509

## Deliberate reproduction

The existing Set-DolbyRoom.ps1 provides Status, On, Off, HalfFeedback and Restore. It pins this endpoint, consumer and reviewed profile bytes; checks other properties; saves durable backups; refuses unrelated state drift; and never selects a provider. With Dolby Atmos for Headphones already selected, On enables the saved native room, Off mutes its output under the same normalization, HalfFeedback shortens decay, and Restore returns to the user's original low-reverb profile. No provider switch or profile write was performed now because retained controls and recording evidence were sufficient for this offline continuation.

Remaining investigation is upstream construction of the optional profile metadata and the effective state override, plus cross-category/cross-distance validation. The practical room control itself is already demonstrated. It should not be conflated with importing arbitrary measured-room FIRs or BRIRs into the recursive native room topology.
