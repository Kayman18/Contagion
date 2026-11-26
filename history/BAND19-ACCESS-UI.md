# BAND19 Access UI

Firmware identity `FM-1_15007498`. This source builds the four-encoder UI and the Access Virus C loading splash on the 512-frame dual-core BAND19 engine.

- K1 to K4 edit the four controls shown on each sound page. Turn ALGO in either direction to navigate all pages. No encoder banks or button combinations are used for page navigation.
- HOME opens Oscillators. EDIT, ENV, LFO and FX open their section. Press a section button again to step through its pages.
- Turn PRESET to browse factory programs. On Factory Presets, K1 changes the bank, K2 browses programs, K3 loads Init, and K4 edits patch volume.
- SEL opens USER without saving. K1 selects a slot, K2 recalls it, and K3 and K4 edit the name. SAVE writes the selected slot.
- GLO opens Performance. Press GLO again for Core Timing, Core Status, Audio Status, Wave Cache and Cache Budget. K2 resets counters on these pages.
- On a UC25HQ80 unit, Core Status shows the flash ID and embedded-wave status. Embedded waves are the normal playback path on this chip and user storage remains enabled.

## Note transitions

This build uses a 32-frame fade for fast amp attacks and voice steals. It preserves the saved envelope, preset format and dual-core routing.

## Build

The firmware build needs the JieLi Linux toolchain and AC79 SDK. Set `JIELI_TOOLCHAIN` and `AC79_SDK` to their directories, then run `bash build-band-firmware.sh`. The package is produced at `app-band/build/virooos-0.74.98.fwsc`.

The RGB565 splash and knob assets are compiled from the checked-in `app-band/access_art.inc`. To regenerate them from the approved square splash art, pass the image path to `python3 tools/generate-access-ui.py` with Pillow installed.

For the simulator, run `python3 base/tools/build.py app-band --host`. Then run `app-band/build/host/virooos_host tests/access-ui.txt` with an existing output directory as the final argument to capture the 240 × 240 screens.
