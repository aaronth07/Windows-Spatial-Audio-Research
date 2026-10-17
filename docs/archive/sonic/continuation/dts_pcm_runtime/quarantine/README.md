> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Private configuration reproducer

`private_layout_probe.cpp` preserves the actual unsupported private configuration caller used in the retained exception trace. It is excluded from the default build and runner. `../build.ps1 -Target quarantine` compiles it separately; this compilation was tested. No installation or system mutation is required.

The historical reproducer invocation has the same positional arguments as `type_probe`: decoder DLL, dependency directory, MP4, metadata endpoint interface, and `private_51`. It can terminate the process with access violation 0xc0000005. The trace is `../private_layout_crash_debug.log`, from a separate CDB process using `../private_layout_crash.cdb`. That historical executable was named type_probe.exe; the current quarantined build is private_layout_probe.exe. See `../private_layout_diagnosis.md` for the exact observed fault and missing contract. This source is retained for analysis, not a supported configuration API or a default test. Do not repeat the same crash without a changed, evidence-supported hypothesis.
