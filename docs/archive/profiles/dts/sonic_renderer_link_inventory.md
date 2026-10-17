> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Windows Sonic renderer linkage inventory

This inventory records read-only file identity, selected ASCII/UTF-16LE string offsets, and PE import/export listings for the installed System32 audio components. It does not infer native behavior, interface relationships, or a rendering path.

`audioeng.dll` and `Windows.Media.Audio.dll` were copied from System32 into `SpatialRendererProfileImport/binaries` because copies were absent. Their installed-source and copied-file versions, lengths, and SHA-256 hashes are in `sonic_renderer_link_inventory.json`; each source/copy pair matched. The same JSON records the exact installed paths, versions, lengths, and hashes for `HrtfApo.dll` and `HrtfDspCpu.dll`.

Strings were searched in raw bytes in both ASCII and UTF-16LE. Offsets are zero-based. Matches observed:

| String | File | Encoding | Byte offset(s) |
|---|---|---|---|
| `HrtfApo.dll` | `audioeng.dll` | ASCII | 1075520 |
| `HrtfApo.dll` | `Windows.Media.Audio.dll` | ASCII | 949982 |
| `HrtfApo.dll` | `HrtfApo.dll` | ASCII | 122698 |
| `HrtfApo.dll` | `HrtfApo.dll` | UTF-16LE | 152128, 152320 |
| `HrtfDspCpu.dll` | `HrtfApo.dll` | UTF-16LE | 95408 |
| `HrtfDspCpu.dll` | `HrtfDspCpu.dll` | ASCII | 2144714 |
| `HrtfDspCpu.dll` | `HrtfDspCpu.dll` | UTF-16LE | 2189048 |
| `CreateHrtfEngineFactory` | `audioeng.dll` | ASCII | 1120912 |
| `CreateHrtfEngineFactory` | `HrtfApo.dll` | ASCII | 122753 |
| `CreateHrtfDspFactory` | `HrtfApo.dll` | ASCII | 95528 |
| `CreateHrtfDspFactory` | `HrtfDspCpu.dll` | ASCII | 2144750 |
| `SpatialSoundDataManagerRpc` | `HrtfApo.dll` | UTF-16LE | 95672 |

No `Windows Sonic` string was found in either searched encoding in the four inspected files. That result applies only to these files and encodings.

`objdump.exe -p` output was limited to each PE Import Tables and Export Tables section. The Function Table/unwind section, data directory, and unrelated PE headers were excluded. The captured listings and exact line counts are embedded in the JSON.

For reproduction, hash and inspect file versions with `Get-FileHash -Algorithm SHA256` and `(Get-Item $path).VersionInfo.FileVersion`. Search the named strings as both `[Text.Encoding]::ASCII.GetBytes($value)` and `[Text.Encoding]::Unicode.GetBytes($value)` in `[IO.File]::ReadAllBytes($path)`. PE import/export sections were obtained with `<local-path>` and captured only between the `The Import Tables` / `The Export Tables` headings and the next `The ...` section heading.

No binaries were executed or loaded. No apps, COM objects, or audio components were activated, and no audio, settings, or registry state was changed.
