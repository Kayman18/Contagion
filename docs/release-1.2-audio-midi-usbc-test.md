This build adds USB-C MIDI note output to the eight-voice Contagion firmware for the M-VAVE FM-1. It also includes stereo USB-C audio recording at 44.1 kHz, 16-bit PCM. The USB editor can still use MIDI and SysEx on the same connection.

| MIDI source | Channel |
| --- | --- |
| Onboard keys | 1 |
| OXI or SIG+ lane A | 2 |
| OXI or SIG+ lane B | 3 |

### Install and test

1. Flash `Contagion-1.2-Audio-and-MIDI-USBC-Test.fwsc` with the FM-1 updater. Its firmware ID is `FM-1_15012308`.
2. Reconnect USB. In Reaper, enable **C8V** under **MIDI Inputs**, then arm a track and choose **Input: MIDI → C8V → All Channels**. Play the keys and start the sequencer to record notes. Select **Contagion 8V** as the stereo audio input for a separate audio recording at 44.1 kHz.
3. From **HOME**, press **GLO** seven times to open **USB MIDI STATUS**. Press and release a key. The page shows USB configuration status and the counts of generated, queued, and completed MIDI events.

All 1,525 factory presets, the 34-table wave cache, 32 USER patch slots, and 8 USER sequence slots are included. Keyboard and sequencer note output passed simulation. USB MIDI reception in Reaper and timing during simultaneous audio capture still need FM-1 hardware verification.

For the stable build, download [Contagion 1.2](https://github.com/Kayman18/Contagion/releases/tag/v1.2.0).
