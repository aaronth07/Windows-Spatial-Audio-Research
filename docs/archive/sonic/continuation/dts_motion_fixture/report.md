> Historical research note. Workspace instructions and artifact references below describe the original experiment; use the public agent-handoff.md for continuation.

> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# DTS motion fixture research

Scope: primary public sources only. No files were downloaded; this is a web-documentation review. The supplied native `DTSXDecoder.dll` result remains separate: its 10 speaker planes / 12 fixed PCM columns do not prove authored moving objects.

## Result

No official public DTS:X fixture was found that documents an authored moving object with a stable identity and known trajectory. There is, however, a directly relevant public ETSI DTS-UHD specification that documents object IDs, associated audio waveforms, position metadata, and a decoder-to-renderer relationship. This supports a specification-level object-decoding path for DTS-UHD Profile 3; it does not prove that the tested DTSXDecoder MFT provides object-isolated PCM, exposes positions, or consumes arbitrary speaker geometry. DTS-UHD's broadcast/streaming Profile 3 should not be silently equated with the tested legacy DTS:X MFT path.

## Primary sources

- ETSI TS 103 491 V1.2.1, DTS-UHD Audio Format

- FFmpeg DTS-UHD demuxer/parser patch thread plus interleaved bed/object audio (`.atmos.audio`). Its CLI guide says DTS:X objects use stream metadata frame by frame and moving objects therefore move in the exported master set; fixed height feeds become bed channels. The source describes the DTS:X private-metadata interpretation as corpus-derived and alternate profiles as experimental. It also documents a DTS-HD HRA lossy-carrier path decoding the fixed height quartet, explicitly not an object path. These are implementation claims in project docs; no binary/source build was run for this inventory. The realtime plugin is a renderer bridge, not a standalone generic decoder ABI; releases list GPL-3.0-or-later for bridge libraries, while the offline CLI is Apache-2.0.
  - CLI decoder/output guide`, so it is an object renderer/decoder alternative for Atmos formats, distinct from DTS. The repository does not identify a tagged version in the material reviewed. The guide says the open-source code license heavily discourages commercial use and requires a Cavern Pro license for unsupported commercial use.

## Inventory conclusion

A public open-source candidate now exists for the requested DTS:X object/position path: harletty's plugin and offline CLI claim frame-by-frame DTS:X object motion from bitstream metadata, with fixed height channels kept separate. Its proprietary metadata parser is experimental, and the plugin is tied to the Omniphony bridge API, so its documented support does not establish compatibility with the tested MFT or a drop-in decoder API. FFmpeg evidence found here covers transport parsing for DTS-UHD only. Cavern/Truehdd provide separate Atmos object-decoding paths (E-AC-3 JOC and TrueHD Atmos respectively); they should not be used to support a DTS claim.
