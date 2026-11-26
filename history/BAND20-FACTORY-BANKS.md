# BAND20 Virus factory banks

This kit uses the supplied BAND20 dual-core source. Each firmware has a replacement factory list compiled from one of the Access Virus SysEx sets. The native USER patch storage and its existing SysEx protocol use the same record schema.

| Firmware model | Labels | Source programs | Included | Arp programs skipped |
| --- | --- | ---: | ---: | ---: |
| Virus A | A–B | 256 | 227 | 29 |
| Virus B | A–F | 768 | 648 | 120 |
| Virus C | A–F | 768 | 650 | 118 |

Choose one firmware from the release folder, in 8V or 12V. Virus C is the default source configuration. Virus A or B changes the preset set. All three use the same VIROOOS engine and supported parameter mapping. Unsupported effects and synthesis features can change how a patch sounds. Factory bank contents are compiled in. The device accepts its existing VIROOOS USER slot SysEx format.

The supplied Virus C A–F files contain the same 768 program payloads as the old C–H reference. Patch names, synth parameters, modulation routes and program indices are identical in that variant. The bank letters follow the supplied dumps, so old C2 is now A2 and old E16 is now C16.

The PRESET encoder browses the installed list. MIDI Bank Select MSB (CC0) uses zero-based bank indices (A=0, B=1, etc.), followed by the original zero-based Program Change number. Skipped arp programs leave gaps.

## Build

Install Node.js, Python 3, the JieLi Linux toolchain, and the AC79 SDK. Set JIELI_TOOLCHAIN and AC79_SDK to the installed directories.

```sh
VIRUS_MODEL=B VOICES=8 ./build-virus-banks.sh
```

VIRUS_MODEL accepts A, B or C. VOICES accepts 8 or 12. VIRUS_BANK_DIR can point to another directory containing the corresponding single-program dump files named Virus-X-Bank-Y.syx. The generator verifies every message length, manufacturer and command, bank, program order and checksum. It filters arp patches and regenerates the parameter, modulation and coverage tables. The script writes these generated files and the voice-count header before building, so run one variant at a time per source tree. Add --host to build the simulator.

See validation/virus-banks for source hashes, firmware validation and simulator logs. Physical FM-1 testing of these bank variants is pending.
