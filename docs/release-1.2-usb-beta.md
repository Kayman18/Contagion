Contagion 1.2 USB BETA adds USB-C recording to the eight-voice M-VAVE FM-1 firmware. The firmware sends its stereo master output as 16-bit PCM at 44.1 kHz. USB MIDI and SysEx remain available for the patch editor, and the analog outputs remain available for monitoring. Use your usual audio output device for computer playback.

### Install and test

1. Flash `Contagion-1.2-USB-BETA.fwsc` with the FM-1 updater. The firmware ID is `FM-1_15012108` and the display shows `CONTAGION 1.2 USB`.
2. Reconnect USB and select **Contagion 8V** as the recording input in your DAW at **44.1 kHz**. Arm a stereo track.
3. Connect the USB editor and change Cutoff during a recording. Play a dense sequence and check peak CPU, XRUNS, and DEADLINE on the FM-1 GLO page.

The beta includes all 1,525 factory presets, the 34-table wave cache, 32 USER patch slots, and 8 USER sequence slots. Lossless splash compression made room for USB audio, leaving 1,188 bytes in the firmware slot and 2,760 bytes of main RAM free.

Package integrity, simulated install and rollback, and the USB capture buffer passed validation. Hardware recording and simultaneous editor use are the focus of this beta. To return to the stable firmware, flash the [Contagion 1.2 release](https://github.com/Kayman18/Contagion/releases/tag/v1.2.0).
