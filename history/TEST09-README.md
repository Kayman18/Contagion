# FM1 Virooos — TEST09: all 64 source waves

Native C partial port of https://virooos.vercel.app/ for the Mvave FM-1.
Install `release/FM1-Virooos-TEST09.fwsc` with the usual FM-1 updater.
This is an FWSC package, not a full-flash M-Flasher BIN.
See `HARDWARE-TEST.txt` for the test sequence.

## What changed

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
before allowing playback. Hardware reconstruction time is not measured yet.

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
| FX | Amp slope, filter slope, pan, split width | Sub level, sub shape, ring level, unused |
| FX again | Noise level, noise colour, pulse widths 1/2 | Oscillator tuning 1/2, oscillator key tracking 1/2 |

To hear the new waves, set Shape 1 and Shape 2 to 0. Shape 64 is saw and shape
127 is pulse; changing the wave number is inaudible in these pure saw/pulse modes.
For pulse-width testing use shape 127 and intermediate widths, such as 48/80.
Tune range is +/-48 semitones. Oscillator key tracking defaults to 100%, centred
on MIDI note 60; 0% holds a fixed pitch. Sub pitch follows oscillator 1.

GLO opens performance; K2 resets counters. Reset AFTER loading before comparing
playback CPU. GLO again opens the unchanged fixed, muted six-stage profiler;
K1 selects its stage. PLAY panics; OCT-/OCT+ changes the onboard keyboard octave.
MASTER sets output level. PRESET/SAVE/ARP/SEQ/REC are unassigned. ENV again
selects its filter bank. LFO and FX are page shortcuts, not implemented effects.

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
TEST09 needs hardware testing. TEST07 user feedback was 51% average / 57% peak;
settings and error counters were not restated. TEST06 confirmed four notes,
sub 37/ring 39, 56% average / 64% peak with no new xruns/deadlines.
TEST08 hardware results have not been reported.

Target application: 216,704 / 581,564 bytes. Normal data+BSS: 29,192 / 98,304 bytes.
Audio pool: 262,260 / 344,064 bytes. Executable SRAM: 7,236 / 24,576 bytes,
including 4,268 bytes of synth/profile kernels. Host synth struct: 1,408 bytes.
FWSC: 610,108 bytes, identity FM-1_15000900, version 0.9.0 / TEST09.

## Building and reproducing checks

```
node tools/generate_tables.cjs
node tools/generate_wave_bank.cjs
python3 tools/pack_wave_bank.py
./tests/run.sh
./build-host.sh
app/build/host/virooos_host tests/test09-cache.txt build/test09-cache
app/build/host/virooos_host tests/test09-install.txt build/test09-install
JIELI_TOOLCHAIN=/absolute/path/to/jieli-linux-toolchains-20260730.1 ./build-firmware.sh
```

Generated tables/bank are included. The toolchain is not bundled. TEST09 links
its r3 software-double runtime for loading only; the audio DSP object is checked
for software-double references. The regular audio arithmetic remains float32.
Four voices, core 0, 44.1 kHz and the 256-frame audio buffer are unchanged.
Historical design and validation detail is retained in `history/TEST08-README.md`.

## Remaining work and provenance

This is not a complete Virus C/web-app port. Oscillator 3, FM, saturation, LFOs,
modulation matrix, arpeggiator, unison, SysEx patches and preset persistence remain.
Effects WASM is captured reference material, not linked. The wrapper still uses
linear velocity, deterministic phase and fixed output gain. Immediate voice
stealing/parameter changes can click; high combined levels/resonance can clip.

Original public deployment assets are in `reference/`, captured 2026-11-12;
URLs/hashes are in `reference/manifest.json`. No original source licence was
established; this checkpoint asserts no redistribution licence for those assets.
Platform copyright/licence notices are preserved. The platform came from the
user's FM1-VStation source; that synth is not used. The wTABLES load-time/cache
architecture informed this loader, but its PCM16 bank codec is not reused.
