# Contagion MIDI controls

All MIDI channels address the same eight-voice engine. During wave loading,
factory program changes and wave requests can replace a pending selection.
Other performance and parameter messages take effect after loading. Incoming
CC values are clamped to the supported range.

## Factory selection and USER patches

MIDI CC0 selects a bank. Values 0–1 select Virus A, 2–7 select Virus B, and
8–13 select Virus C. Program Change uses the original zero-based program
number within that bank. Program numbers omitted by the build are skipped.

The USB MIDI SysEx protocol transfers the 16 stored USER patches. The editor
uses this protocol for full patch edits and CC messages for live controls.
[USB editor setup](web-editor/README.md)

## Original-source parameter assignments

| CC | Original parameter | Native range |
|---|---|---|
| 5 | portamento | 0–127 |
| 10 | panorama | 0–127 |
| 17 | osc1Shape | 0–127 |
| 18 | osc1PulseWidth | 0–127 |
| 19 | osc1Wave (wavetable selection) | 0–63 |
| 20 | osc1Semitone | 0–127 |
| 21 | osc1Keyfollow | 0–127 |
| 22 | osc2Shape | 0–127 |
| 23 | osc2PulseWidth | 0–127 |
| 24 | osc2Wave (wavetable selection) | 0–63 |
| 25 | osc2Semitone | 0–127 |
| 26 | osc2Detune | 0–127 |
| 27 | fmAmount | 0–127 |
| 28 | osc2Sync | 0–1 |
| 29 | osc2FiltEnvAmt | 0–127 |
| 30 | fmFiltEnvAmt | 0–127 |
| 31 | osc2Keyfollow | 0–127 |
| 33 | oscBalance | 0–127 |
| 34 | subOscVolume | 0–127 |
| 35 | subOscShape | 0–1 |
| 36 | oscVolume | 0–127 |
| 37 | noiseVolume | 0–127 |
| 38 | ringModVolume | 0–127 |
| 39 | noiseColor | 0–127 |
| 40 | cutoff | 0–127 |
| 41 | cutoff2 | 0–127 |
| 42 | filter1Resonance | 0–127 |
| 43 | filter2Resonance | 0–127 |
| 44 | filter1EnvAmt | 0–127 |
| 45 | filter2EnvAmt | 0–127 |
| 46 | filter1Keyfollow | 0–127 |
| 47 | filter2Keyfollow | 0–127 |
| 48 | filterBalance | 0–127 |
| 49 | saturationCurve | 0–14 |
| 51 | filter1Mode | 0–3 |
| 52 | filter2Mode | 0–3 |
| 53 | filterRouting | 0–3 |
| 54 | filtEnvAttack | 0–127 |
| 55 | filtEnvDecay | 0–127 |
| 56 | filtEnvSustain | 0–127 |
| 57 | filtEnvSustainTime | 0–127 |
| 58 | filtEnvRelease | 0–127 |
| 59 | ampEnvAttack | 0–127 |
| 60 | ampEnvDecay | 0–127 |
| 61 | ampEnvSustain | 0–127 |
| 62 | ampEnvSustainTime | 0–127 |
| 63 | ampEnvRelease | 0–127 |
| 67 | lfo1Rate | 0–127 |
| 68 | lfo1Shape | 0–67 |
| 69 | lfo1EnvMode | 0 = CYCLE; 1–127 = ENVELOPE |
| 70 | lfo1Mode | 0 = VOICE; 1–127 = GLOBAL |
| 72 | lfo1Keyfollow | 0–127 |
| 73 | lfo1TrigPhase | 0–127 |
| 75 | lfo1Osc2 | 0–127 |
| 76 | lfo1PulseWidth | 0–127 |
| 77 | lfo1Resonance | 0–127 |
| 78 | lfo1FilterGain | 0–127 |
| 79 | lfo2Rate | 0–127 |
| 80 | lfo2Shape | 0–67 |
| 81 | lfo2EnvMode | 0 = CYCLE; 1–127 = ENVELOPE |
| 82 | lfo2Mode | 0 = VOICE; 1–127 = GLOBAL |
| 83 | lfo2Contour | 0–127 |
| 84 | lfo2Keyfollow | 0–127 |
| 85 | lfo2TrigPhase | 0–127 |
| 86 | lfo2Shape12 | 0–127 |
| 87 | lfo2FmAmount | 0–127 |
| 88 | lfo2Cutoff1 | 0–127 |
| 89 | lfo2Cutoff2 | 0–127 |
| 90 | lfo2Panorama | 0–127 |
| 91 | patchVolume | 0–127 |
| 93 | transpose | 0–127 |

## Retained controls and compatibility aliases

| Message | Action |
|---|---|
| Note on/off | Play/release; velocity zero is note-off |
| Pitch bend | Configured up/down ranges and curve |
| CC7 | Channel volume: squared gain, default 127 |
| CC11 | Expression: squared gain, default 127 |
| CC64 | Sustain: 64+ on, below 64 off |
| CC71 | Resonance 1: native alias, 0–127 |
| CC74 | Cutoff 1: native alias, 0–127 |
| CC120 | Panic: clear voices and sustain |
| CC123 | All notes off: release subject to sustain |

CC71 and CC74 select resonance and cutoff. MIDI clock is unsupported.

Patch volume starts at 90 for factory and INIT patches. A value of 0 mutes the patch.
Portamento 0 is off. Panorama 64 is center; endpoints retain the original curve.
Envelope sustain-time controls use the native slope mapping: 64 holds sustain.
CC7/11 have no displayed controls. Panic retains parameter values and controller
levels. Store edits in a USER slot to recall them later.

Wave CC19 and CC24 request oscillator waves. Values above 63 clamp to 63. Repeating the current selection does
not reload; new requests during loading replace pending selections. OSC3 keeps
its independent wave selection; slave/built-in modes follow oscillator 2.
These are load operations, not continuous wavetable-position modulation.

CC69/81 select envelope mode. Both explicitly enable their LFO,
including from OFF: 0 selects CYCLE and any nonzero MIDI value selects ENVELOPE.
Select OFF on the device. Phase is preserved; switching does not retrigger notes.
Other LFO controls do not enable an OFF LFO by themselves. CC70/82 select shared/global mode for LFO1/2. It does not enable an OFF LFO. Shared rate ignores key tracking and per-voice detuning. Trigger phase 0 free-runs in CYCLE; nonzero phase retriggers the shared LFO on each note-on. Envelope mode retriggers even at phase 0. Shared LFOs run while idle, and selecting VOICE/GLOBAL preserves phase. Live envelope-mode changes preserve phase and do not retrigger notes. The LFO offers all 62 wavetable
shapes 6–67 (WAVE 2–63); shapes above 67 clamp to 67. CC71/74 retain filter
aliases; they do not control LFO1 contour or oscillator-1 pitch. LFO3 has no new
CC assignments. Incoming MIDI clock is not supported.

## Keyboard mode

CC94 -> keyMode: 0 POLY, 1 MONO 1, 2 MONO 2, 3 MONO 3, 4 MONO 4, 5 HOLD.
Values above 5 clamp to 5. Released chords remain latched
until the next phrase starts; CC123 releases them and PLAY/panic clears them.
Mode changes clear notes, held/deferred stacks and pedal state. See README
for retrigger/glide differences. CC5 still controls portamento.
