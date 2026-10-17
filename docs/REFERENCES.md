# Primary references

The implementation evidence is version-pinned; documentation below describes the public contracts or methods. Private ABI observations do not become supported APIs through these links.

- Microsoft: [Render spatial sound using spatial audio objects](https://learn.microsoft.com/en-us/windows/win32/coreaudio/render-spatial-sound-using-spatial-audio-objects).
- Microsoft: [ISpatialAudioClient](https://learn.microsoft.com/en-us/windows/win32/api/spatialaudioclient/nn-spatialaudioclient-ispatialaudioclient).
- Microsoft: [Object position and coordinate system](https://learn.microsoft.com/en-us/windows/win32/api/spatialaudioclient/nf-spatialaudioclient-ispatialaudioobject-setposition).
- Microsoft: [WASAPI loopback recording](https://learn.microsoft.com/en-us/windows/win32/coreaudio/loopback-recording).
- Microsoft: [Capture GetBuffer timestamps and flags](https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudiocaptureclient-getbuffer).
- Microsoft: [IAudioClock GetPosition](https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclock-getposition).
- Microsoft: [Spatial stream GetService](https://learn.microsoft.com/en-us/windows/win32/api/spatialaudioclient/nf-spatialaudioclient-ispatialaudioobjectrenderstreambase-getservice).
- Microsoft: [Audio processing objects](https://learn.microsoft.com/en-us/windows-hardware/drivers/audio/audio-processing-object-architecture).
- Microsoft: [MFT input stream buffering information](https://learn.microsoft.com/en-us/windows/win32/api/mftransform/ns-mftransform-mft_input_stream_info).
- ETSI: [TS 103 491 V1.2.1, DTS-UHD](https://www.etsi.org/deliver/etsi_ts/103400_103499/103491/01.02.01_60/ts_103491v010201p.pdf).
- SOFA: [SimpleFreeFieldHRIR](https://www.sofaconventions.org/mediawiki/index.php/SimpleFreeFieldHRIR) and [GeneralFIR](https://www.sofaconventions.org/mediawiki/index.php/GeneralFIR).
- AES: [Simultaneous measurement of impulse response and distortion with a swept-sine technique](https://aes.org/publications/elibrary/elibrary-page/?id=10211).
- Pulkki: [Virtual sound source positioning using vector base amplitude panning](https://www.audiolabs-erlangen.de/resources/aps-w23/papers/sap_Pulkki1997.pdf).
- Equalizer APO: [Configuration reference](https://sourceforge.net/p/equalizerapo/wiki/Configuration%20reference/).
- Audited open-source alternative: [Harletty Bridge v0.9.0](https://github.com/harletty/harletty-bridge/releases/tag/v0.9.0), source revision `9556bcedfd6e71461d49fd3f03fcfdf87351b67f`. The audit distinguishes legacy DTS syntax from DTS-UHD and does not assert verified moving-object coverage.

Additional links retained inside the anonymized archived notes are historical source pointers. Their presence is not a claim that this release refreshed every external page or verified every third-party assertion.
