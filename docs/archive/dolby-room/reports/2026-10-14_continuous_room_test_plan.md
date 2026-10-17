> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Room control on an existing object stream

The completed room tests used a fresh native stream per capture. This bounded follow-up checks whether the practical control also affects a stream that stays open. It is covered by the user's selected virtual endpoint authorization. No capture-engine or native DLL changes are required.

Start with exact personalized API94b3… and Dolby selected. Save fresh all-six-key registry and selected/active format observations. The already-tested command control saves durable API/registry/User backups before its first write and checks the exact expected current state. Keep all defaults, routing, formats and other properties unchanged.

One native dynamic object and one paired recording remain open for25 positions: five repeated groups, each containing a transition guard then front/left/back/right. Use the same48kHz,-34dBFS one-second sweep,.25-second lead and two-second tail. At guard indices5/10/15/20, call On/Off/HalfFeedback/Restore respectively. Those are the already native-tested52f3…/36d2…/a7a5…/94b3… payloads. Only normal Dolby Init User PID1 changes. Verify the full saved-key state after each command. Require each control to complete within2.5seconds of its guard beginning; exclude the guard sweeps entirely from response comparisons.

The capture engine verifies original94 at stream start and finish; the orchestrator separately records the intentional intermediate hashes and timing. No pause/reopen/Reset occurs between groups. Compare the four stable positions in each group, retaining one common stereo-field lag and original gain. A null existing-stream effect, stream invalidation or discontinuity is a meaningful result and prevents claiming immediate control on existing playback.

The entire test has a130-second deadline. On failure, stop only this experiment-owned stream and restore the saved personalized baseline. Final proof requires94/04, all six registry keys byte-identical, Dolby formats unchanged and raw recordings/logs retained. No provider toggle, Access launch or service/device operation is included.
