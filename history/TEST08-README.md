# FM1 Virooos — TEST08 firmware and source

Partial native C port of https://virooos.vercel.app/ for the Mvave FM-1.
Install `release/FM1-Virooos-TEST08.fwsc` using the FM-1 firmware updater.
See `HARDWARE-TEST.txt` for the test sequence.

TEST01 and TEST02 hardware feedback: CPU average 42%, peak 48%, xruns 0, deadlines 0.
TEST07 user feedback: CPU average 51%, peak 57%; settings and error counters were not restated.
TEST08 passes host and package checks but still needs hardware measurements.
Four voices, core 0 and the 256-frame audio buffer are unchanged.

## TEST08 oscillator controls

Added independent pulse widths, oscillator tuning (-48 to +48 semitones), and
oscillator key tracking using the captured source mappings. Defaults preserve
TEST07 pitch and waveform behavior. Sub pitch follows oscillator 1.

From HOME press FX twice for OSC EXTRAS / NOISE. Top-bank K1/K2 remain noise
level/colour; K3/K4 set pulse widths 1/2. SEL selects the lower bank: K1/K2 tune
oscillators 1/2, K3/K4 set key tracking 1/2. Tracking is centred on MIDI note 60;
100% follows the keyboard, 0% holds a fixed pitch, negative values invert it.
Displayed tracking is rounded toward zero; maximum is approximately 196.875%.
Pulse width is audible when oscillator shape includes pulse. At pure pulse and
width 127 the source duty cycle reaches zero; use intermediate widths to test.

11,760 comparisons against the original voice control engine cover extreme notes,
tuning, widths, tracking and pitch bend; maximum absolute error 7.27e-8.
Native +/-2-semitone linear bend behavior is retained (reference bendScale=0).
The three existing envelope, sub/ring and noise scripted WAVs are byte-identical
to TEST07. Sanitizer, UI, profiler, SRAM-placement and package checks pass.
The fixed muted profiler retains its previous patch. TEST08 hardware CPU and
stability have not yet been measured.

## TEST07 source noise generator

TEST06 is the confirmed working hardware baseline: four notes, sub 37/ring 39,
CPU average 56%, peak 64%, with no new xruns/deadlines during the user's test.
The muted FULL test was also reported at 56%/64%. TEST07 was subsequently reported at 51% average/57% peak; exact settings and
error counters were not restated.

Added the original noise level curve and colour filter. FX opens stereo/sources;
press FX again to open OSC EXTRAS / NOISE. K1 sets noise level, K2 colour (0–127).
Colour 64 is unfiltered white noise; below 64 uses the original low-pass colour
curve with gain compensation; above 64 attenuates low frequencies. Noise enters
filter 2 in SPLIT routing and the common input in the other routes. Level defaults
to zero and colour to 64, so existing sounds retain their default behavior.

The browser's Math.random() is replaced by an independent per-voice 32-bit xorshift
stream, seeded from note and allocation age. This is not the browser's exact
random sequence. The source amplitude scaling (.744), colour coefficients,
filter recurrence, level curve and routing are retained. Noise state pauses while
its level is zero, and colour changes preserve filter history as the source does.

Tests compare the actual captured JS noise class with native C using an identical
controlled random input: 851,968 samples, all 128 colours at 22.05/44.1/48 kHz and
live colour changes, maximum absolute error 4.74e-7. Combined source-voice routing
comparisons now cover 655,360 outputs including noise (maximum error 8.05e-7).
Sanitizer stress and exact block-partition tests include noise. Normal scripted
noise-off WAVs are byte-identical to TEST06; the muted profiler remains silent.
The profiler retains its TEST06 fixed setup with noise OFF for comparable readings.

Four voices, core 0, buffer 256, SRAM kernels and the division optimization remain.

## Retained TEST06 oscillator optimization

The saw/pulse correction previously calculated an edge fraction on every sample.
A conservative two-increment distance check now skips that division when the
oscillator is certainly too far from a discontinuity. The original division,
comparison and BLEP arithmetic remain unchanged near edges and at hard sync.
The original source waveforms and algorithms are retained.

The retained TEST05 step and optimized step produce bit-identical output AND
state across 2,098,208 comparisons covering both waves, all shape values, varied
widths/increments, sync and neighboring discontinuity boundaries. The normal
scripted sub/ring WAV is byte-identical to the saved TEST05 demo. Existing source
reference tests, sanitizer tests and the muted profiler UI checks pass.

For the fixed profiler chord over one second, source-level oscillator edge
fraction divisions fall from 352,800 to 3,663 (98.96% fewer). This count excludes
sub-oscillator, setup and sync divisions and does NOT predict total CPU savings.
TEST06 hardware feedback is recorded above; TEST05 has no matching profiling readings. The profiler and its six stages remain available unchanged.

Test instructions: GLO twice; K1 selects a stage, K2 resets statistics. Compare
FULL and OSC first against TEST05, then report FILTERS/ENVELOPES/CONTROL/IDLE and
CPU MHZ if available. The benchmark is intentionally muted and needs no keys.

## Retained TEST05 hardware profiler

TEST04 hardware feedback: four active voices, sub level 17 and ring level 13,
CPU average 101%, peak 110%, xruns/deadlines 525. PLAY returned voices to zero
and CPU settled at 5%. The reported fifth voice was corrected by the user to four.
Moving audio code to SRAM did not resolve overload in that test. Earlier TEST01/02
42%/48% readings lack confirmed matching chord/patch conditions, so a regression
between versions is not established.

Press GLO to open performance, then GLO again for the muted profiler. K1 selects
FULL, OSC / SUB / RING, FILTERS, ENVELOPES, CONTROL or IDLE. K2 resets counters;
PLAY exits to normal performance and silences notes. Other page buttons also exit.

Every stage uses a freshly initialized, separate benchmark synth: notes 48, 55,
60, 64 at velocity 100, sub 17, square shape, ring 13, otherwise default patch.
Envelopes begin at steady sustain. Your normal parameter values are preserved.
Incoming MIDI notes are discarded during profiling; onboard note-ons are ignored.
Audio output is digitally muted in every profiling stage, including FULL.

FULL runs the actual render in 32-frame pieces. Other stages isolate actual source,
filter-routing or envelope routines; CONTROL performs cached filter-control work
at the normal 32-frame cadence. FILTERS uses deterministic square test inputs.
IDLE omits DSP. Display includes CPU percentage, peak, last audio-block duration
in microseconds, reported CPU MHz, xruns/deadlines, voice count and block size.
CPU MHz depends on the platform counter measurement and may be unavailable (0).

These percentages include platform overhead and have different loop and mixing
costs. They are comparative workloads, not additive timing slices of a live patch.
CONTROL measures steady cached coefficients, not worst-case moving envelopes.
The benchmark never changes voice count, clock, sample rate or buffer size.
The host profiler test verifies digital silence, stage selection, exit behavior,
full-render state equivalence, stage isolation and repeated sanitizer workloads.
The TEST02/03 normal-audio scripts remain byte-identical to TEST04.

## Retained TEST04 RAM placement

User reported TEST03 at four voices: average 104%, peak 110%, xruns/deadlines 905,
with similar load when sub/ring were enabled. The earlier 42%/48% readings were
reported for TEST01/02; exact playing conditions were not independently verified.
This suggests investigating the common render path; flash/cache sensitivity is
an unconfirmed hypothesis, not an established root cause.

The target render, oscillator, sub, filter, envelope, source-mix and routing
routines now execute from the platform's existing copied `.voice_hot` SRAM
section. Small arithmetic helpers are forced inline so they do not branch back
to flash. The app uses one indirect dispatch per audio block to cross the chip's
direct-call distance limit. Wave/control tables remain in flash.

Linked kernels occupy 4,388 bytes including the profiler; total executable-RAM use is 7,356 of 24,576.
The existing startup copy and PC guard cover this region. Placement is checked
by `tools/check_hot_placement.py` with the toolchain objdump and linked ELF.
Four voices, core 0, buffer 256 and DSP algorithms remain unchanged. Both the
TEST02 envelope script and TEST03 sub/ring script produce byte-identical host
WAVs versus TEST03. TEST04 hardware results are recorded above; the overload remained.

## Retained from TEST03

- Source square/triangle sub oscillator, one octave below oscillator 1, with
  source volume curve and square-wave polyBLEP correction.
- Source ring modulation of the two raw oscillator signals, with its original
  level curve and half of the signal sent to each filter input in SPLIT routing.
- FX lower bank (SEL): K1 sub level, K2 square/triangle, K3 ring level.
  Both levels default to zero. The TEST02 scripted audio is byte-identical with
  the new sources disabled. Sub phase pauses while its level is zero, as in source.
- Four voices, core 0 and buffer 256 remain unchanged. The original patch-volume
  gain model is still pending; current oscillator-section gain remains unity.

## Retained from TEST02

- Source filter ADSR, separate envelope amounts and polarities for both filters,
  linked cutoff option, independent key tracking and a shared key base.
- Independent filter modes and resonance; source SERIAL 4, SERIAL 6, PARALLEL
  and SPLIT routes, with source filter-balance curves. SERIAL 6 uses four poles
  on filter 1 and two on filter 2.
- Source rising/falling sustain slopes for the amplitude and filter envelopes.
- Stereo pan and split-route stereo width, using the source pan law.
- Filter coefficient caching and a reciprocal calculated at coefficient updates
  instead of every audio sample. Actual hardware CPU benefit is not yet measured.
- Filter envelope controls update every 32 audio samples, preserving timing
  across arbitrary render block boundaries.

## Existing core

Four pooled voices; MIDI note on/off, velocity, sustain, +/-2-semitone bend,
CC74 cutoff 1, CC71 resonance 1, CC120 panic and CC123 all notes off.
Two source-derived oscillators per voice with shape morph, corrected saw/pulse,
wave indices 0 and 1, detune and oscillator-2 hard sync. Source-derived filters
support low-pass, high-pass, band-pass and notch. Onboard keyboard, octaves,
master volume and performance display are included. No DSP heap allocation.
USB/TRS MIDI share one voice pool and a global sustain controller.

## Controls

ALGO changes page. SEL/SELECT toggles the active bank of four parameters.
K1–K4 edit the selected bank, left-to-right then down.

| Button/page | Top bank | Bottom bank |
|---|---|---|
| HOME: oscillators | Shape 1, shape 2, oscillator balance, detune | Wave 1, wave 2, sync, cutoff 2 |
| EDIT: filters | Cutoff 1, resonance 1, mode 1, routing | Cutoff 2, resonance 2, mode 2, filter balance |
| ENV: envelopes | Amplitude attack, decay, sustain, release | Filter attack, decay, sustain, release |
| LFO: filter modulation | Envelope amount 1, amount 2, polarity 1, polarity 2 | Cutoff link, key follow 1, key follow 2, key base |
| FX: envelope/stereo | Amplitude slope, filter slope, pan, split width | Sub level, sub shape, ring level, unused |
| FX again: oscillator extras | Noise level, noise colour, width 1, width 2 | Tune 1, tune 2, key tracking 1, key tracking 2 |

ENV opens the amplitude bank; pressing ENV again selects the filter bank.
LFO and FX are shortcuts to the pages above; no LFO or effects engine is present.
FX again opens noise level/colour on K1/K2. GLO opens performance. K2 resets CPU, peak, xrun/deadline, lost-event and clipping
counters. PLAY panics; OCT-/OCT+ change keyboard octave; MASTER changes volume.
PRESET, SAVE, ARP, SEQ and REC are unassigned. Slopes at 64 hold a fixed sustain;
lower/higher values fall/rise. Split width applies to SPLIT routing.
Filter envelope amounts default to zero so the initial sound stays familiar.

## Limits and remaining work

This is not a complete Virus C or full web-app port. Only two of 64 source waves
are included. Oscillator 3, FM, saturation, LFOs,
modulation matrix, arpeggiator, unison, SysEx patches and persistence are pending.
The captured effects WASM is reference material and is not linked.

The voice wrapper still uses linear velocity, deterministic oscillator phase and
fixed output gain rather than the complete original patch behavior. Source route
and pan algorithms are present, but full-patch audio equivalence is not claimed.
Voice stealing is immediate; parameter changes other than master volume are not
smoothed and can click. Extreme resonance can exceed full scale: PCM output is
clamped and GLO counts clipped frames. Start with moderate volume/resonance.
Bluetooth and USB audio are disabled. Analog audio and USB MIDI are available.

## Validation

`./tests/run.sh` runs C99/Node.js reference comparisons and native sanitizer tests.
The captured JavaScript is executed independently for 2,594,128 scalar comparisons:

| Comparison | Values | Maximum absolute error |
|---|---:|---:|
| Oscillator wave/morph/saw/pulse | 16,384 | 5.15e-8 |
| Oscillator tuning, tracking, pulse width and bend | 11,760 | 7.27e-8 |
| Hard sync | 2,048 | 4.76e-8 |
| Filter impulses, all modes/pole counts | 65,536 | 1.67e-6 |
| Fixed-sustain ADSR | 20,000 | 6.70e-6 |
| Source routing, modes, balances, sub, ring and noise | 655,360 | 8.05e-7 |
| Noise colours and colour changes at three sample rates | 851,968 | 4.74e-7 |
| Sub waveforms, phases and frequencies | 131,072 | 1.55e-7 |
| All 128 sustain-slope settings | 768,000 | 6.66e-5 |
| Filter envelope/polarity/link/keytracking/timing | 72,000 | 1.81e-5 |

These are bounded component/state comparisons, not complete patches. ASan/UBSan
checks pass for voice limits/steals, sustain, zero-velocity note-off, panic,
parameter bounds, filter/envelope stress, stereo isolation and exact render-block
partition invariance. LeakSanitizer is disabled under this environment's ptrace;
the DSP engine uses no heap. Host synth state is 1,328 bytes, excluding tables,
platform buffers/stacks/UI and other memory.

Build the headless simulator with `./build-host.sh`, then run, for example:

```
mkdir -p build/test03-ui
app/build/host/virooos_host tests/test03-ui.txt build/test03-ui
```

Both existing UI/hardware-input-path scripts and the TEST02 and TEST03 UI scripts pass.
Their reports and screenshots are in `validation/`; a host-generated test audio
file is in `demo/`. Host performance counters are stubs, not hardware measurements.

## Firmware build

Set `JIELI_TOOLCHAIN` to the root containing `pi32v2/bin/clang`, then run
`./build-firmware.sh`. Packaging SDK subset: `build-deps/sdk`. The base platform
came from the user's FM1-VStation-RC1 archive; its synth/application are not used.
The JieLi Linux toolchain is not bundled; see `base/tools/get_toolchain.sh`.

Target checks pass: SRAM code has no calls, no unsafe 64-bit multiply/load pattern,
no soft-double routines, and register access stays in the platform BSP.
Application 325,264 / 581,564 bytes; normal .data+.bss RAM 22,900 / 98,304 bytes;
pool 16,388 / 344,064 bytes. Stacks and retained state use separate regions.
FWSC is 610,108 bytes, identity `FM-1_15000800`, release 0.8.0 / TEST08.
The independent package validator verifies directory/payload CRCs and exact app
and loader contents, and rejects three corrupted variants.

This is an FWSC updater package, not a full-flash M-Flasher BIN.
Toolchain archive SHA-256:
`a1545cddbc451a06bac43eb5cd96d831470c42a97c5c0b1f98879a20fc7647bd`.

## Source provenance

Captured public deployment assets on 2026-11-12: `reference/processor.js`,
`reference/index.js`, `reference/virus_fx.wasm`; exact URLs/hashes are in
`reference/manifest.json`. No original repository/licence was found during the
initial inspection; this checkpoint asserts no redistribution licence for those
upstream assets. Platform copyright/licence headers are retained in `base/`.
Recreate DSP tables with `node tools/generate_tables.cjs`.
