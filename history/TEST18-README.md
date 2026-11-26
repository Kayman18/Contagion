# FM1 Virooos — TEST18: LFO 2 stereo pan

Native C partial port of https://virooos.vercel.app/ for the Mvave FM-1.
Install `release/FM1-Virooos-TEST18.fwsc` with the usual FM-1 updater.
This is an FWSC package, not a full-flash M-Flasher BIN.
See `HARDWARE-TEST.txt` for the test sequence.

## TEST18 LFO 2 stereo-pan modulation

From HOME press LFO six times, then SEL. K4 is now LFO2 PAN AMT, replacing the
duplicate contour control. LFO 2 contour remains on its filter page (HOME ->
LFO five times, top K3). The oscillator page's lower bank is now rate, shared
BPM, shape and pan amount.

Pan amount defaults to +0 (raw 64). It follows the original source expression:
base panorama + 2*(amount-64)*LFO 2. The original pan law and endpoint handling
are retained. Each voice has its own pan target and smoothly moves to it over
32 samples. The first target on a new note is applied immediately, matching the
source's initial gain snap. Modulation works in serial, parallel and split
routing; split mode keeps the existing left/right filter spread around the
moving pan position. Each side clamps separately at the stereo edges.

Changing pan amount to +0 or switching LFO 2 OFF slews back to base pan over one
control interval, then returns to the previous static-pan path. With the new
amount unused, existing static pan behavior is preserved. This implements the
pan component of the source's gain interpolation; its full velocity/patch-volume
model remains unported. Native output scaling is unchanged.

936,960 source comparisons cover stereo audio and pan coefficients across all
routes and filter modes, positive/negative amounts, pan/spread edges and live
LFO timing. Maximum error is below 8.95e-6. Tests verify return to base pan,
finite stress output and exact render-block partition invariance. All thirteen
previous recordings are byte-identical with pan amount neutral; the wave bank
is unchanged. TEST18 needs
hardware listening and CPU checks. Listen in stereo to assess this feature.

TEST17 feedback: no CPU change reported; exact values and error counters were
not restated. TEST15/16 likewise reported no CPU change. Last explicit values
remain TEST14: 56% average / 63% peak, zero xruns/deadline misses.

## TEST17 LFO 2 oscillator shape, FM and tempo divisions

From HOME press LFO six times for LFO 2 / OSCILLATORS:

| Bank | K1 | K2 | K3 | K4 |
|---|---|---|---|---|
| Top | Shape modulation amount | FM modulation amount | LFO 2 clock division | LFO 2 mode |
| SEL | LFO 2 rate | Shared internal tempo BPM | LFO 2 shape | Contour |

Shape modulation uses the original ge curve: (amount-64)*127/128 times LFO 2.
It adds to both oscillator shapes and clamps them to 0–127. This morphs between
the selected cached wave, saw and pulse. It does not select a different wave
number, reload the cache or decompress audio. Shape gains update at control
rate, preserving oscillator phase, pulse width, table choice and pending BLEP
correction. Unchanged modulation skips this work. Base shapes 32 and 96 with
amount +32 give a useful starting test.

FM modulation uses the original signed amount mapping, 2*(amount-64)*LFO 2,
added to base FM amount, velocity contribution and filter-envelope contribution
before clamping and applying the original FM-depth curve. Displayed +0 is
neutral. A positive amount can produce FM even when base FM is zero. Shape
modulation also updates the original oscillator-1 FM normalization factor.
Both new amounts default to +0; LFO 2 remains OFF by default.

LFO 2 now has the same 21 source clock divisions as LFO 1, using their shared
internal tempo of 63–190 BPM (default 120). Clock OFF retains free-rate behavior;
clocked mode bypasses key tracking and per-voice rate variation. Changing rate
or clock division preserves phase. This does not add MIDI-clock/transport sync.
LFO 2 pan modulation, mono/global mode and wavetable LFO shapes remain absent.

733,728 new source comparisons cover all 128 amounts, shape boundaries, FM
contributions, neutral restoration, live clocked/one-shot operation and LFO 1
PWM coexistence. Largest scaled error is below 1.65e-5. Sanitizer stress and
render-block partition checks combine these destinations with filter, pitch,
noise and sync modulation. All twelve previous scripted WAVs are byte-identical
at the new controls' defaults; wave-bank bytes are unchanged. TEST17 still needs hardware CPU/listening checks.

TEST16 feedback: no CPU change reported; exact values and error counters were
not restated. The last explicit measurements remain TEST14: 56% average / 63%
peak with zero xruns/deadline misses. TEST15 also reported no CPU change.

## TEST16 independent LFO 2 filter modulation

From HOME press LFO five times for LFO 2 / FILTER CUTOFF:

| Bank | K1 | K2 | K3 | K4 |
|---|---|---|---|---|
| Top | LFO 2 rate | Shape | Contour | Trigger phase |
| SEL | Filter 1 cutoff amount | Filter 2 cutoff amount | LFO 2 mode | Rate key tracking |

LFO 2 has independent per-voice state and the original six standard shapes,
contour, trigger, +/-2% rate variation and one-shot behavior. Rate and key
tracking use the same source curves as LFO 1. Defaults are rate 64, sine,
contour +0, trigger 0 DEG, key tracking 0, both cutoff amounts +0 and mode OFF.
CYCLE enables bipolar modulation; ONE SHOT uses the source's unipolar output.
OFF pauses the LFO and removes its cutoff contributions. Retrigger a note to
restart a completed one-shot.

Cutoff amounts use the original signed nonlinear curve, displayed -64 to +63.
They add independently to each filter's cutoff after envelope and key tracking,
then clamp to 0–128. Filter 2's cutoff link is applied before LFO modulation,
so each filter keeps its own LFO amount even when linked. Set base cutoffs near
70 and amounts +32/-32 to hear opposing movement. Existing envelopes, LFO 1
resonance/gain modulation and pitch/PWM modulation remain active.

LFO 2 uses a separate deterministic random stream; it does not advance the
existing noise, start-phase or LFO 1 random streams. Like LFO 1 it runs at the
source's 32-sample control cadence, with the previous LFO value used before
advancing the generator. This build does not yet expose LFO 2 clock divisions,
shape/FM/pan destinations, mono/global mode or wavetable shapes.

681,216 new source comparisons cover cutoff curves with envelope, key tracking
and cutoff linking, plus live dual-LFO operation and filter coefficients. Largest
scaled live error is below 1.31e-5; largest absolute cutoff error is below
0.000211 control units. Sanitizer stress and block-partition checks include both
LFOs together. All eleven previous scripted WAVs are byte-identical with LFO 2
OFF; wave-bank bytes are unchanged. TEST16 hardware CPU and listening checks are pending.

TEST15 user feedback: no CPU change. Exact CPU values and error counters were
not restated. Last explicit measurements remain TEST14: 56% average / 63% peak,
zero xruns and deadline misses.

## TEST15 LFO 1 tempo divisions and key tracking

From HOME, press LFO four times for LFO 1 / TIMING:

| Bank | K1 | K2 | K3 | K4 |
|---|---|---|---|---|
| Top | Clock division | Internal tempo BPM | LFO key tracking | LFO mode |
| SEL | Free rate | Trigger phase | Shape | Contour |

Clock OFF preserves the existing free-rate behavior. Key tracking defaults to
0 and uses the source formula 2^(((note-60)/12)*(amount/128)), amount 0–127.
MIDI note 60 is the reference; higher notes run faster and lower notes slower.
This tracks the played note, not pitch bend or oscillator tuning. At 127 the
scaling is close to doubling per octave. It does not alter oscillator pitch
unless an LFO pitch destination is enabled.

The 21 original clock divisions run from 1/64 to 16 bars, including dotted (D)
and triplet (T) settings. Internal tempo defaults to 120 BPM, range 63–190 BPM,
using the original raw mapping 63 + parameter. A bar means four quarter notes.
1/4 at 120 BPM is a 0.5-second LFO cycle; 1 BAR is two seconds. Clock mode
bypasses free rate, key tracking and the +/-2% per-voice rate deviation.

This is internal tempo timing, not incoming MIDI clock or transport sync.
LFOs remain per voice; clock mode matches their rate, not necessarily their
phase. Trigger phase still applies on note-on. Changing tempo, division or
key tracking does not reset phase or restart a completed one-shot. Start a new
note to restart one-shot mode. All rate calculations occur on note/parameter
changes; the existing 32-sample LFO update cadence is retained.

345,216 new comparisons cover all 128 MIDI notes and key-tracking values,
all clock divisions and tempo values, plus live voice timing and parameter
changes in cycle and one-shot modes. Largest scaled rate error is below 3.8e-7;
largest live phase/value error is below 6.5e-6. All ten previous scripted WAVs are byte-identical to TEST14 at default timing;
the bank is unchanged. TEST15 hardware measurements are pending. TEST14 is confirmed at 56% average / 63% peak CPU, zero xruns and
deadline misses with filter modulation enabled.

## TEST14 LFO 1 filter destinations

From HOME, press LFO three times. The new LFO 1 / FILTERS page provides:

| Bank | K1 | K2 | K3 | K4 |
|---|---|---|---|---|
| Top | LFO resonance amount | LFO filter gain amount | LFO rate | LFO mode |
| SEL | Base resonance 1 | Base resonance 2 | Base cutoff 1 | Base cutoff 2 |

Both amounts default to displayed +0 (raw 64). Resonance uses the original
signed nonlinear curve and affects both filters, clamping each result to 0–127.
Filter gain uses the original asymmetric signed amount curve and
max(0, 1 + amount * LFO) before the filters in every routing mode. This is not a
master output-volume control; filter tails can continue after its input reaches
zero. One-shot mode retains the source's unipolar LFO, which can raise gain above
its cycling-mode range. Turning LFO OFF restores base resonance and unity gain.

Fractional resonance in the four-pole filter now uses the source's exponential
damping curve between anchors. Integer resonance values retain their previous
coefficients. Static filter settings still use cached coefficients; changing
resonance recalculates them at the existing 32-sample control cadence.

The new tests compare 1,634,304 coefficient, gain and stereo audio values against
the captured source across all 128 amount settings, six LFO values, four filter
modes and all four routes. Maximum error: 1.20e-6. Sanitizer stress and block
partitioning tests include the new modulation. TEST14 hardware CPU/listening
checks remain pending. All nine previous scripted WAVs are byte-identical to
TEST13 with the new amounts neutral; the wave bank is unchanged. TEST13 is confirmed at 52% average / 59% peak, zero xruns
and deadline misses with LFO enabled.

## TEST13 LFO 1

Press LFO twice from HOME to open LFO 1 / OSCILLATORS. Top controls are rate,
shape, contour and trigger phase. SEL selects oscillator 1 pitch, oscillator 2
pitch, pulse-width amount and mode: OFF / CYCLE / ONE SHOT. Defaults are OFF
and neutral destination amounts. Signed amounts are not semitone counts.

The original six standard shapes are implemented: sine, triangle, saw, square,
sample-and-hold and sample-and-glide. Rate follows the original nonlinear curve
(about 283 seconds per cycle at 0, 1.09 Hz at 64, 91.55 Hz at 127), with the
original +/-2% per-voice rate variation. Contour changes the shape. Trigger FREE
starts from the fresh voice's random phase; fixed settings select a start phase.
ONE SHOT follows the source's unipolar 1+shape output and holds its completed
value. Start a new note to retrigger it. OFF pauses the LFO and removes its
contributions. Each new native voice has its own deterministic random stream;
this does not reproduce the browser's Math.random sequence or full legato logic.

Pitch and PWM use the original source curves and 32-sample control timing,
including reading the previous LFO value before advancing the LFO. Pitch combines
with tuning, bend and oscillator 2's pitch envelope. PWM affects both oscillators;
use pulse shapes to hear it. Pitch-band changes use cached waves without loading.
External MIDI clock sync, wavetable shapes, global/mono operation, LFO 2/3 and modulation-matrix connections remain absent.

Validation: 2,480,808 comparisons against the captured source cover all six
shapes, 128 contours, trigger/one-shot behavior, pitch/PWM routing and live control
timing. Largest error is 1.52e-5. Sanitizer stress includes LFO plus FM, pitch
envelopes and sync; arbitrary render-block partitioning remains exact. All eight
previous scripted WAVs are byte-identical to TEST12 with LFO OFF. Wave-bank bytes
are unchanged. UI and firmware checks pass. TEST13 was subsequently confirmed at 52% average / 59% peak with zero xruns
and deadline misses; TEST11/12-specific listening checks remain outstanding.

## TEST12 oscillator start phase

FX -> SEL -> K4 now controls OSC START:

- RESET is the native compatibility default: oscillator 1, oscillator 2 and sub
  start at phase zero, preserving TEST11's sound and all existing test patches.
- Fixed angles use the original oscInitPhase values 1–127. Oscillator 1/sub start
  at zero; oscillator 2 starts at value/128 of a cycle. The screen displays whole
  degrees truncated toward zero; 32/64/96 give exactly 90/180/270 degrees.
- RANDOM uses the original random-mode phase relationships: independent oscillator
  1/2 start phases, and sub phase equal to half oscillator 1's phase (He.reset).

The native RESET setting is internal value 128, an extension for compatibility;
the original source's value 0 means RANDOM. Random starts use a separate per-note
xorshift stream seeded from note/allocation age. They are reproducible after a
restart and do not reproduce the browser's Math.random sequence. The audible
noise generator's state is not advanced or changed by phase selection.

Changing OSC START affects only subsequent note starts; it leaves currently
sounding oscillator, envelope and control state unchanged. Phase selection and
random draws occur at note start, with no new per-sample processing. The existing
native hard-steal/retrigger policy is retained; this is not a port of the source's
complete mono/legato voice allocation. Oscillator 3 is still absent.

Validation: 1,161 phase values across all source settings and three note ages
match the actual captured Voice.noteOn exactly using controlled random inputs;
RESET is checked separately. ASan/UBSan checks verify phase bounds, unchanged
noise RNG, unchanged sounding audio after a setting change, exact repeatability
for every fixed setting and varied random retriggers. All seven existing WAVs
are byte-identical to TEST11 at RESET. Firmware package/placement checks pass.
The wave bank is unchanged and is reused without rewriting a matching bank.

TEST12 still needs hardware listening/CPU checks. TEST11's pitch-envelope feature
is included but also has no hardware feedback yet. The last confirmed hardware
baseline remains TEST10: 56% average / 62% peak, zero xruns/deadline misses.

## TEST11 oscillator 2 pitch envelope

Added the original osc2FiltEnvAmt mapping: the shared filter envelope can raise
or lower oscillator 2's pitch. The nonlinear source amount curve is retained,
including the endpoints. It updates at the existing 32-sample control cadence.
Pitch bend, tuning, detune and key tracking remain active; oscillator 1 and sub
pitch are not changed by this control. FM operates on the resulting oscillator-2
increment, and hard sync continues to follow oscillator 1.

From HOME press FX three times, then SEL. The lower bank now provides oscillator
2 pitch-envelope amount, filter attack, filter decay and filter sustain. The
amount displays -64 to +63, with +0 neutral (raw parameter 64). Its display is
an amount control, not a semitone count. Filter release remains on ENV's lower
bank. These are the same filter-envelope controls used for filter and FM
modulation, so changing their timing affects every enabled destination.

The oscillator keeps its phase while pitch moves. Band selection follows pitch
using the already-loaded tables, without decompression, wave loading or muting.
An unchanged pitch-envelope value skips the exponential and band update. The
unclamped base increment is retained so a downward sweep from a high note can
correctly return below the source's 0.45 increment ceiling.

Validation adds 15,360 comparisons covering all 128 amount settings, low/high
notes, pitch bend, envelope levels and wave bands (maximum error 1.22e-7), plus
36,000 live envelope/increment comparisons through attack, decay and release
(maximum error 1.57e-5). Sanitizer stress and render-block invariance include
pitch-envelope modulation combined with FM and sync. All six existing scripted
WAVs, including FM, match TEST10 byte-for-byte with the pitch amount at zero.
The wave-bank bytes are unchanged; the fixed muted profiler also remains silent.

TEST10 is the current confirmed hardware baseline: 56% average / 62% peak CPU,
zero xruns and deadline misses. The exact FM source was not restated in the user
feedback. TEST11 hardware CPU and listening checks are still outstanding.

## TEST10 oscillator FM

Added the four original internal FM sources: positive triangle, triangle,
oscillator 1 waveform and noise. FM changes oscillator 2's phase increment,
including negative increments at high bipolar depth, using the source arithmetic.
Hard sync retains its original oscillator-1 crossing and fractional reset logic.
External/auxiliary FM inputs are not exposed because those audio inputs are absent.

From HOME press FX three times for OSCILLATOR FM. K1 controls amount (0–127),
K2 selects the source, K3 sets the filter-envelope contribution and K4 velocity
contribution. K3/K4 display -64 to +63; zero is neutral (raw parameter 64).
The source adds twice these signed amounts times envelope level/velocity to the
FM amount, clamps it to 0–127, then applies the original depth curve (0 to 16).
Envelope FM updates at the existing 32-sample control cadence. A positive velocity
amount adds depth with higher note velocity. Amount 0 alone does not disable FM
if envelope/velocity contributions are positive: reset both to displayed zero.

Noise FM works with audible noise level at zero. If both are enabled, they share
one colour-filtered noise sample per voice per sample. Noise colour remains on
FX twice. The noise generator pauses when neither audible noise nor noise FM
needs it. Defaults are FM amount 0 and neutral envelope/velocity contributions.

No wave-bank update is needed over TEST09: its bank bytes are unchanged. Startup
still reconstructs the RAM cache. Five existing sub/ring, envelope, noise, tuning
and wave-selection scripted recordings are byte-identical with FM at default.
The fixed profiler patch remains FM-off. TEST10 subsequently measured 56% average / 62% peak with zero errors.

New validation:
- 3,456 depth/envelope/velocity comparisons against original Voice.control:
  maximum error 7.75e-7, limit 3e-6.
- 18,432 oscillator-2 increment comparisons captured from original Voice.render,
  covering all four FM modes, shapes, depths and noise colours: maximum 2.42e-8.
- 16,384 original oscillator comparisons covering negative, zero, positive and
  multi-cycle increments plus fractional hard sync: maximum error 1.07e-7.
- Noise-state checks verify exactly one generator step when noise and noise FM
  are both active. Sanitizer stress includes FM, envelope/velocity amounts,
  hard sync and pitch extremes; arbitrary render-block partitioning stays exact.
- UI checks cover all modes, modulation amounts, four voices, page cycling and
  panic. Firmware CRC/size/placement checks pass. No hardware result is claimed.

## Retained TEST09 wave loader

Both oscillators now select source waves 0–63. Each wave retains all fourteen
2049-sample pitch-band tables. Two shared oscillator caches serve all four voices;
there is no per-voice decompression or table reconstruction in the audio loop.

The original float64 harmonic coefficients are stored losslessly in per-wave LZ4
blocks in the wav/data area. The bank is 90,426 bytes including its directory and
CRCs. The earlier 44,207-byte estimate used whole-bank zlib compression; this
implementation uses the small, bounded LZ4 decoder and independently loadable
blocks. No coefficients or float32 waveform samples are quantised to PCM16.

The original sine constants, harmonic accumulation order, float32 band snapshots
and final normalisation are retained. Reconstruction uses software float64 only
in the main-loop loader while playback is muted. All 1,835,904 reconstructed
float32 samples match the original source tables byte-for-byte on the host.
The hardware also checks each completed table set against its expected CRC
before allowing playback. The user reported approximately 1–2 seconds for wave loading in TEST09.

The two decoded caches occupy 229,488 bytes of audio-pool RAM. A separate 16,384-byte
accumulator is reused for reconstruction. A waveform is reconstructed once when
selected, not when pressing a note or changing its pitch. Existing pulse width,
tuning, key tracking, noise, sub, ring, envelopes, filters and profiler are retained.

## First boot and wave changes

The FWSC contains an installer copy of the compact bank. First boot installs it
at 0x97000, replacing the previous contents of sectors 0x97000–0xADFFF. This is
wav/data space; settings at 0xDC000 and above are not touched. Switching back to
another firmware that uses that data requires reinstalling its matching bank.
A matching bank is reused without flash writes on later boots. An incomplete or
mismatched bank is reinstalled from the embedded copy on the next boot. The
installer copy remains in the app so no separate full-image flash is needed.

Wait for LOADING WAVES - MUTED to disappear. Wave changes silence active notes,
load/reconstruct the selected tables and check their CRCs. Press notes again after
loading; notes received during loading are discarded. Rapid selection changes
are supported; only the current selection is committed. This is a muted bank
switch, not a continuous waveform-morph implementation. A failed install or CRC
shows WAVE DATA ERROR and keeps audio silent.

## Controls

ALGO cycles pages; SEL/SELECT switches the bank of four controls. K1–K4 edit
left-to-right, top then bottom. HOME always selects the top bank.

| Page | Top bank | Bottom bank via SEL |
|---|---|---|
| HOME | Shape 1, shape 2, oscillator balance, detune | Wave 1 (0–63), wave 2 (0–63), sync, cutoff 2 |
| EDIT | Cutoff 1, resonance 1, mode 1, routing | Cutoff 2, resonance 2, mode 2, filter balance |
| ENV | Amp attack, decay, sustain, release | Filter attack, decay, sustain, release |
| LFO | Filter envelope amounts 1/2, polarities 1/2 | Cutoff link, filter key tracking 1/2, key base |
| LFO again | Rate, shape, contour, trigger phase | Oscillator 1 pitch, oscillator 2 pitch, pulse width, LFO mode |
| LFO third press | Resonance amount, filter gain amount, rate, mode | Base resonance 1/2, base cutoff 1/2 |
| LFO fourth press | Clock division, internal BPM, key tracking, mode | Free rate, trigger phase, shape, contour |
| LFO fifth press | LFO 2 rate, shape, contour, trigger | Cutoff amounts 1/2, LFO 2 mode, key tracking |
| LFO sixth press | LFO 2 shape amount, FM amount, clock, mode | LFO 2 rate, shared BPM, shape, pan amount |
| FX | Amp slope, filter slope, pan, split width | Sub level, sub shape, ring level, oscillator start phase |
| FX again | Noise level, noise colour, pulse widths 1/2 | Oscillator tuning 1/2, oscillator key tracking 1/2 |
| FX third press | FM amount, source, filter-envelope amount, velocity amount | Oscillator 2 pitch-envelope amount, filter attack, decay, sustain |

To hear the new waves, set Shape 1 and Shape 2 to 0. Shape 64 is saw and shape
127 is pulse; changing the wave number is inaudible in these pure saw/pulse modes.
For pulse-width testing use shape 127 and intermediate widths, such as 48/80.
Tune range is +/-48 semitones. Oscillator key tracking defaults to 100%, centred
on MIDI note 60; 0% holds a fixed pitch. Sub pitch follows oscillator 1.

GLO opens performance; K2 resets counters. Reset AFTER loading before comparing
playback CPU. GLO again opens the unchanged fixed, muted six-stage profiler;
K1 selects its stage. PLAY panics; OCT-/OCT+ changes the onboard keyboard octave.
MASTER sets output level. PRESET/SAVE/ARP/SEQ/REC are unassigned. ENV again
selects its filter bank. LFO cycles filter modulation, LFO 1 oscillators, LFO 1 filters, LFO 1 timing, LFO 2 filters and LFO 2 oscillators; FX cycles its three pages.

## Validation

- All 64 reconstructed table sets: 1,835,904 float32 samples byte-identical.
- 5,505,024 oscillator comparisons across all waves and pitch bands, three shapes
  and fractional hard sync: maximum absolute error 2.05e-7, limit 2e-6. Dyadic
  increments isolate oscillator arithmetic from long-term float phase drift.
- Retained component/routing/noise/envelope/control comparisons: 2,594,128 values.
- ASan/UBSan: reconstruction, corruption/truncation/bounds rejection and dynamic
  engine voice limits, steals, sustain, panic, stress and render-block invariance.
- UI: waves 62/63, four-note playback, cancellation/rapid wave changes, notes
  ignored while loading, releases and panic. Startup installs the bank; reboot
  reuses it without writes; corrupting its header triggers successful repair.
- Existing sub/ring, tuning, envelope and noise WAVs match TEST08 byte-for-byte
  with matched startup timing. Without aligning the initial wait, two scripts
  differ by one UI-triggered panic block; DSP output is otherwise unchanged.
- Muted profiler output remains all zero. SRAM placement and package CRC checks
  pass, including three deliberately corrupted FWSC variants.

Host checks establish code/data behavior, not physical FM-1 CPU or loading time.
TEST09 is now the confirmed hardware baseline: 53% average / 59% peak CPU, zero
xruns/deadline misses, and approximately 1–2 seconds to load a wave selection.
TEST10 was confirmed at 56% average / 62% peak with zero errors. TEST11 still needs hardware testing. TEST07 user feedback was 51% average / 57% peak;
settings and error counters were not restated. TEST06 confirmed four notes,
sub 37/ring 39, 56% average / 64% peak with no new xruns/deadlines.
TEST08 hardware results have not been reported.

Target application: 228,008 / 581,564 bytes. Normal data+BSS: 31,144 / 98,304 bytes.
Audio pool: 262,260 / 344,064 bytes. Executable SRAM: 11,932 / 24,576 bytes,
including 8,964 bytes of synth/profile kernels. Host synth struct: 2,400 bytes.
FWSC: 610,108 bytes, identity FM-1_15001800, version 0.18.0 / TEST18.

## Building and reproducing checks

```
node tools/generate_tables.cjs
node tools/generate_wave_bank.cjs
python3 tools/pack_wave_bank.py
./tests/run.sh
./build-host.sh
app/build/host/virooos_host tests/test18-lfo2-pan.txt build/test18-lfo2-pan
app/build/host/virooos_host tests/test17-lfo2-osc.txt build/test17-lfo2-osc
app/build/host/virooos_host tests/test16-lfo2.txt build/test16-lfo2
app/build/host/virooos_host tests/test15-lfo-timing.txt build/test15-lfo-timing
app/build/host/virooos_host tests/test14-lfo-filter.txt build/test14-lfo-filter
app/build/host/virooos_host tests/test13-lfo.txt build/test13-lfo
app/build/host/virooos_host tests/test12-start-phase.txt build/test12-start-phase
app/build/host/virooos_host tests/test11-pitch-env.txt build/test11-pitch-env
app/build/host/virooos_host tests/test10-fm.txt build/test10-fm
app/build/host/virooos_host tests/test09-cache.txt build/test09-cache
app/build/host/virooos_host tests/test09-install.txt build/test09-install
JIELI_TOOLCHAIN=/absolute/path/to/jieli-linux-toolchains-20260730.1 ./build-firmware.sh
```

Generated tables/bank are included. The toolchain is not bundled. TEST09 links
its r3 software-double runtime for loading only; the audio DSP object is checked
for software-double references. The regular audio arithmetic remains float32.
Four voices, core 0, 44.1 kHz and the 256-frame audio buffer are unchanged.
Historical design and validation detail is retained in `history/`.

## Remaining work and provenance

This is not a complete Virus C/web-app port. Oscillator 3, external/auxiliary FM inputs, saturation, remaining LFO functions,
modulation matrix, arpeggiator, unison, SysEx patches and preset persistence remain.
Effects WASM is captured reference material, not linked. The wrapper still uses
linear velocity and fixed output gain. Start phase is now selectable. Immediate voice
stealing/parameter changes can click; high combined levels/resonance can clip.

Original public deployment assets are in `reference/`, captured 2026-11-12;
URLs/hashes are in `reference/manifest.json`. No original source licence was
established; this checkpoint asserts no redistribution licence for those assets.
Platform copyright/licence notices are preserved. The platform came from the
user's FM1-VStation source; that synth is not used. The wTABLES load-time/cache
architecture informed this loader, but its PCM16 bank codec is not reused.
