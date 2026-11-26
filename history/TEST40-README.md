# FM1 Virooos — TEST40: 40 wavetable LFO shapes

Install `release/FM1-Virooos-TEST40.fwsc` with the usual updater. This is an FWSC package, not a full-flash M-Flasher BIN.

TEST40 expands the exact original wavetable LFO selection from 32 to 40 shapes, independently available to all three LFOs. Standard shapes remain 0–5; wavetable shapes 6–45 display WAVE 2–41. WAVE 34–41 are new. Values above 45 clamp to 45. The remaining 22 source shapes (46–67) are not implemented yet.

LFO1: HOME -> LFO twice -> K2 SHAPE; SEL -> K4 CYCLE/ENVELOPE.
LFO2: HOME -> LFO five times -> K2 SHAPE; SEL -> K3 CYCLE/ENVELOPE.
LFO3: HOME -> LFO seven times -> K2 SHAPE; SEL -> K4 ON.
Set nonzero destination depth. Changing shape leaves an OFF LFO disabled. CC68/80 select LFO1/2 shapes 0–45; LFO3 selection remains on-device.

All 81,960 float32 samples match the captured source exactly, including the compiled target ELF bytes. The 40 tables occupy 327,840 bytes of mapped application flash. RAM usage and WAV partition are unchanged; there is no runtime generation or decompression. Playback retains the existing interpolation and LFO cadence. Physical CPU usage has not been measured for this build.

Validation: the complete suite and ASan/UBSan checks pass. 2,749,480 LFO values were compared, with exact table knots and maximum interpolation error 0.0000148164. All 38 prior WAV renders match TEST39 rebuilt on the same host byte-for-byte. The historical TEST39 fixture now selects shape 37 explicitly instead of using 127, whose clamp changed; TEST40 separately verifies the new 127 -> 45 clamp. All three LFOs, four voices, panic, MIDI selection and limits pass host integration checks.

Target: application 576332/581564 bytes (5232 free), normal RAM 83800/98304, pool 327836/344064, executable RAM 15408/24576. FWSC 610108 bytes, identity FM-1_15004000. Package CRC/payload, hot/cache placement and exact LFO table placement checks pass.

Emulator at 240 MHz: TEST40 booted, WAVE 20, WAVE 38 and WAVE 41 displayed; Selecting WAVE 20 preserved OFF; CYCLE was then selected, OSC1/OSC2 pitch depths +9/+6 were accepted, four voices displayed, and panic cleared voices to zero. Firmware xruns/deadlines remained zero during this short check. Emulator timing is not a physical FM-1 CPU measurement; audible quality and hardware stress testing remain outstanding.

This remains a partial native port. See the historical notes below for earlier features and limitations; current range and sizes above supersede TEST39 values.

## Historical TEST39 notes

# FM1 Virooos — TEST39: 32 wavetable LFO shapes

Install `release/FM1-Virooos-TEST39.fwsc` with the usual updater. This is an FWSC package, not a full-flash M-Flasher BIN.

TEST39 expands the exact original wavetable LFO selection from 16 to 32 shapes, independently available to all three LFOs. Standard shapes remain 0–5; wavetable shapes 6–37 display WAVE 2–33. WAVE 18–33 are new. Values above 37 clamp to 37. The remaining 30 source shapes (38–67) are not implemented yet.

LFO1: HOME -> LFO twice -> K2 SHAPE; SEL -> K4 CYCLE/ENVELOPE.
LFO2: HOME -> LFO five times -> K2 SHAPE; SEL -> K3 CYCLE/ENVELOPE.
LFO3: HOME -> LFO seven times -> K2 SHAPE; SEL -> K4 ON.
Set nonzero destination depth. Changing shape leaves an OFF LFO disabled. CC68/80 select LFO1/2 shapes 0–37; LFO3 selection remains on-device.

All 65,568 float32 samples match the captured source exactly, including the compiled target ELF bytes. The 32 tables occupy 262,272 bytes of mapped application flash. RAM usage and WAV partition are unchanged; there is no runtime generation or decompression. Playback retains the existing interpolation and LFO cadence. Physical CPU usage has not been measured for this build.

Validation: the complete suite and ASan/UBSan checks pass. 2,199,584 LFO values were compared, with exact table knots and maximum interpolation error 0.0000148164. All 37 prior WAV renders match TEST38 rebuilt on the same host byte-for-byte. The historical TEST38 fixture now selects shape 21 explicitly instead of using 127, whose clamp changed; TEST39 separately verifies the new 127 -> 37 clamp. All three LFOs, four voices, panic, MIDI selection and limits pass host integration checks.

Target: application 510764/581564 bytes (70800 free), normal RAM 83800/98304, pool 327836/344064, executable RAM 15408/24576. FWSC 610108 bytes, identity FM-1_15003900. Package CRC/payload, hot/cache placement and exact LFO table placement checks pass.

Emulator at 240 MHz: TEST39 booted, WAVE 20 and WAVE 33 displayed; WAVE 33 remained OFF until CYCLE was selected, OSC1 pitch depth +9 was accepted, four voices displayed, and panic cleared voices to zero. Firmware xruns/deadlines remained zero during this short check. Emulator timing is not a physical FM-1 CPU measurement; audible quality and hardware stress testing remain outstanding.

This remains a partial native port. See the historical notes below for earlier features and limitations; current range and sizes above supersede TEST38 values.

## Historical TEST38 notes

# FM1 Virooos — TEST38: first 16 wavetable LFO shapes

Native C partial port of https://virooos.vercel.app/ for the Mvave FM-1.
Install `release/FM1-Virooos-TEST38.fwsc` with the usual FM-1 updater.
This is an FWSC package, not a full-flash M-Flasher BIN.
See `HARDWARE-TEST.txt` for the test sequence.

## TEST38 wavetable LFO shapes

Adds the first 16 original wavetable LFO shapes to all three LFOs, independently.
The six standard shapes remain 0–5. New values 6–21 display WAVE 2 through
WAVE 17, matching source table indices (shape minus 4). Values above 21 clamp
to 21; original shapes 22–67 are still pending. This is a partial expansion.

LFO1: HOME -> LFO twice -> K2 SHAPE; SEL -> K4 CYCLE or ENVELOPE.
LFO2: HOME -> LFO five times -> K2 SHAPE; SEL -> K3 CYCLE or ENVELOPE.
LFO3: HOME -> LFO seven times -> K2 SHAPE; SEL -> K4 ON.
Set destination depth as usual: changing shape does not enable an OFF LFO.
CC68 and CC80 now accept 0–21 for LFO1 and LFO2. LFO3 remains device-controlled.
LFO1/2 positive contour compresses the waveform phase range as in the original;
negative contour leaves these tables unchanged. Existing cycle/one-shot,
trigger-phase, clock division and key-tracking behavior is retained.

All 32,784 float32 table samples are exact, generated from the captured source
by tools/generate_lfo_waves.cjs. Tables occupy 131136 bytes of mapped application
flash, not oscillator RAM or the WAV partition. There is no runtime generation
or decompression. Table playback uses linear interpolation at the existing LFO
control cadence. Its physical CPU cost needs measurement; no CPU saving claimed.

Tests: 1,099,792 values compared with the original, including exact table knots,
all contour settings, phase wrapping, cyclic and one-shot output. Maximum error
outside exact knots is 0.000006503. Full suite including ASan/UBSan passes.
The UI script exercises all three wavetable LFOs, MIDI selection, shape limits,
four voices and panic. Target package CRC/payload and SRAM/cache checks pass.
All 36 prior WAVs match TEST37 rebuilt on this same host byte-for-byte. The
TEST35 script now sends shape 5 explicitly instead of 127 (which previously
clamped to 5); new upper-bound behavior is tested in TEST38. Some archived demo
WAVs differed on this restored host, so rebuilt TEST37 is the regression baseline.

The 240 MHz browser emulator boots TEST38, displays WAVE 2, enables CYCLE with
oscillator-pitch depth, runs four notes and clears them with panic. Displayed
xruns/deadlines were zero. External MIDI and sound quality were not verified
in the browser, and emulator CPU is not a physical FM-1 measurement.

Target: app 379612 / 581564 bytes; normal data+BSS 83800 / 98304;
wave pool 327836 / 344064; executable RAM 15408 / 24576 (hot kernels 12440).
FWSC 610108 bytes; identity FM-1_15003800, version 0.38.0 / TEST38.

The TEST37 kit was restored before editing because the active workspace had
reverted to TEST04. Source, tests and generated assets below include TEST37.
Historical sections describe their own checkpoint, not current feature limits.

## TEST37 HOLD keyboard mode

Adds original key mode 5 (HOLD): released notes stay latched. When all physical
keys have been released, the next note starts a new phrase and releases the
previously latched notes through their normal envelopes. Overlapping keys build
a chord, up to the existing four-voice limit. Repeated notes retrigger using the
existing native poly allocation. Portamento follows the existing poly behavior.

HOME -> FX nine times -> SEL -> K3 now selects POLY, MONO 1–4, or HOLD.
CC94=5 selects HOLD; values above 5 clamp to 5. PLAY/panic clears everything;
CC123 releases all notes. Changing key mode clears notes and pedal state as in
TEST36. Sustain-pedal note-offs become latched when the pedal is released;
starting a phrase while the pedal is down does not release pedal-sustained notes
that have not yet become latched. No arpeggiator is added.

The implementation adds only event-time bookkeeping (16 bytes in VCSynth),
with no new per-sample work or changes to the wave bank. Normal RAM rises by
32 bytes after target alignment. All 35 previous WAVs remain byte-identical.

Checks: 1,080 deterministic events and 520,632 state values match the captured
source. The suite covers chord replacement, repeated notes, note-on velocity
zero, sustain ordering, all-notes-off, and all 128 keys. During the 128-key
stress section only bookkeeping is compared because native voice stealing
already differs from the original. Real DSP tests verify finite output,
block-size invariance, three voices held across key releases and replacement
by one held voice. Full previous suite, ASan/UBSan, UI integration, target build,
executable-RAM/cache placement and FWSC checks pass.

Browser emulator at 240 MHz boots TEST37, displays HOLD, retains a three-key
chord after release, replaces it with one note and clears it with panic.
Displayed xruns/deadlines were zero, but the browser reported audio-buffer
underruns. This is control/voice validation, not physical CPU or listening proof.

Target: app 248328 / 581564 bytes; normal data+BSS 83800 / 98304;
wave pool 327836 / 344064; executable RAM 15292 / 24576 (hot kernels 12324).
FWSC 610108 bytes; identity FM-1_15003700, version 0.37.0 / TEST37.

Earlier sections below describe their individual historical checkpoints;
TEST37 supersedes their statements that HOLD is unavailable.

## TEST36 mono keyboard modes

Adds original keyboard modes 1–4, last-note priority, return to held notes,
ordered sustain-pedal releases and CC94. Default POLY retains four voices.

| Mode | Envelope behavior | Portamento between phrases |
|---|---|---|
| POLY (0) | Independent voices | Existing poly behavior |
| MONO 1 | Retrigger each note | Yes |
| MONO 2 | Retrigger each note | No |
| MONO 3 | Legato while notes overlap | Yes |
| MONO 4 | Legato while notes overlap | No |

All mono modes glide between overlapping notes when portamento is nonzero.
Portamento zero disables glide. Returning to a held note retains the current
voice velocity, matching the original. Retrigger preserves oscillator/filter
state and envelope level; legato also preserves envelope and LFO progression.

Controls: HOME -> FX nine times -> SEL -> K3 KEY MODE, K4 PORTAMENTO.
CC94 selects 0–4. Changing modes clears sounding notes, held-note stacks and
pedal state: this is a native adaptation, not the original lazy mode switch.
The original HOLD mode 5 remains unimplemented; higher CC94 values clamp to 4.
Key tracking 1 remains on HOME -> FX twice -> SEL -> K3.

Validation: 35,160 keyboard-state values across 879 events per mode compared
against the captured source; maximum pitch error 0.00001926 from float32
rounding. This comparison isolates keyboard policy, not full audio identity.
Real render tests cover phase/filter/envelope continuity, one-voice limits,
finite output and exact block-versus-single-sample rendering. ASan/UBSan,
full host suite, device UI script, target build and package checks pass.
All 34 previous WAVs are byte-identical in default poly mode. The lossless
wave bank is unchanged. Native amplitude/pan smoothing remains unchanged.

TEST36 boots in the 240 MHz browser emulator. MONO 3 is selectable, a four-key
chord uses one voice, and panic clears it. Displayed xruns/deadlines were zero.
These checks do not establish physical CPU use or audible quality.

Target: app 247800 / 581564 bytes; normal data+BSS 83768 / 98304;
wave pool 327836 / 344064; executable RAM 15292 / 24576 (hot kernels 12324).
FWSC is 610108 bytes; identity FM-1_15003600, version 0.36.0 / TEST36.

## TEST35 LFO MIDI automation

Adds 18 original-source assignments for LFO1/2 rate, standard shape, key tracking,
trigger phase, oscillator-2 pitch/PW, resonance/filter gain, LFO2 contour,
oscillator shape/FM, both cutoffs and pan. Full numbers are in MIDI-CONTROLS.md.
There are now 65 source mappings through the engine, two source wave mappings
through the loader, and the two retained native CC71/74 filter aliases.

These controls do not enable an OFF LFO. Select CYCLE or ENVELOPE using its
existing mode control first. LFO1: HOME -> LFO twice -> SEL -> K4. LFO2:
HOME -> LFO five times -> SEL -> K3. Source mono/global and envelope-mode CCs
69/70/81/82 remain unsupported because native modes are not equivalent.
Source CC71 contour and CC74 oscillator-1 pitch remain unavailable at those
numbers: the port keeps its resonance/cutoff aliases. Those LFO settings remain
available on the device. Shapes clamp to the six native standard shapes 0–5;
source wavetable LFO shapes are not added. MIDI clock is not added.

The existing parameter path and display feedback are reused. No voice DSP,
per-sample work, wave-bank data or audio-buffer change is introduced.

Checks: 25,728 MIDI/direct-parameter state and stereo-render comparisons across
OFF/CYCLE/ENVELOPE modes pass. All source mapping numbers are verified. The UI
script checks both enabled LFOs, controller/display/knob continuity, four notes,
shape limits and retained filter aliases. All 33 previous WAVs, including the
muted loader recording, are byte-identical. Full host suite, target build,
SRAM/cache placement and FWSC checks pass. Final builds were rechecked after
correcting build ordering to ensure the new map was compiled.

Final TEST35 boots in the 240 MHz browser emulator. LFO1 CYCLE and oscillator-2
pitch modulation can be set from the controls; four-key input activates four
voices and panic clears them. Displayed xruns/deadlines were zero. The browser
reported audio-buffer dropouts earlier in the session. External MIDI automation,
audio listening and physical CPU were not verified there; host tests cover the
MIDI paths. Hardware results remain outstanding.

## TEST34 MIDI wave selection

Original CC19 selects oscillator 1's wave; CC24 selects oscillator 2's wave.
Native range is 0–63; incoming 64–127 clamps to 63. Values update on the device
and subsequent knob changes continue from the received selection. This brings
the app to 49 original-source assignments plus two native filter aliases.

Both CCs use the same mute/acknowledge/load/publish transaction as front-panel
wave selection. Actual synth wave parameters change only after the requested
cache set is ready. A different wave stops voices and mutes while loading;
notes must be played again afterward. Repeating the current request is a no-op
and does not cut held notes or restart loading. New wave requests are accepted
while loading and the latest request wins. Other MIDI remains ignored during
loading; all MIDI remains ignored in muted profiler mode.

Oscillator 3's slave/built-in modes continue to follow oscillator 2's wave.
Its independent wave mode retains its own selection when CC24 changes. Bank,
lossless reconstruction, three cache slots and audio buffer are unchanged.
No decompression is added to the audio callback. The engine-only vc_cc API
intentionally does not handle wave CCs: the app owns cache transactions.

Checks pass for clamping, rapid replacement, same-wave four-note continuity,
MIDI-to-knob continuation, notes ignored while loading, independent oscillator-3
cache retention and playback afterward. All 538,368 frames of the muted host
recording are exactly zero; the subsequent playback recording contains audio.
All 31 prior WAV regressions are byte-identical. Full host suite, target build,
SRAM/cache placement and FWSC checks pass. Source CC19/24 names are verified.

Browser emulator at 240 MHz boots TEST34. Knob selection changes wave 1 from
0 to 7, shows LOADING WAVES - MUTED and returns to ready. A subsequent chord
activates all four voices; panic clears them. Firmware counters displayed zero
xruns/deadlines. The browser warned about real-time load during reconstruction;
this does not establish physical CPU/loading time or audio quality. External
MIDI wave requests were tested in the host path, not in the browser session.

## TEST33 expanded synth MIDI automation

Adds 44 assignments for oscillator shapes, pulse widths, semitones/key tracking,
detune, FM, sync, oscillator balance/volume, sub/noise/ring, both filters,
saturation, routing, amplifier/filter envelopes and transpose. There are now
47 original-source parameter CC mappings plus retained native CC71/74 aliases.
See MIDI-CONTROLS.md for exact assignments and ranges. Channel volume,
expression, sustain and panic continue independently of those 49 mappings.

All mapped parameters use the existing parameter-update path and TEST32 display
feedback. Knobs continue from received values. Enum values clamp to the native
parameter maximum (e.g. filter modes 0–3 and saturation 0–14), not a scaled
0–127 selector. Wave selection and LFO-mode mappings remain unimplemented.
CC71/74 deliberately keep native resonance/cutoff rather than source LFO roles.

Checks: 6,272 MIDI/direct-parameter state and stereo-render comparisons pass;
47 mapping numbers match the captured source. All 30 previous WAV recordings
are byte-identical. The UI script tests rapid shape updates, knob continuation,
four active notes, oscillator sources/FM, both filters/routing, envelopes,
transpose and retained filter aliases. Full host suite, target build, memory
placement and FWSC checks pass. Audio DSP and buffer remain unchanged.

Browser emulator at 240 MHz boots TEST33 and shows four active voices after a
four-key chord. Panic clears them. Displayed xrun/deadline counters were zero
in this short check. This is not a hardware CPU result or an audio listening
validation. External MIDI automation in the browser was not exercised; its
parameter behavior and display feedback were tested through the host MIDI path.

## TEST32 MIDI parameter control and display feedback

Adds original-source CC5 portamento, CC10 panorama and CC91 patch volume.
Values are raw 0–127, matching the corresponding controls. The existing native
CC74 cutoff and CC71 resonance aliases remain; these two numbers differ from
the original source's LFO mappings. This is a partial MIDI map, not full original
MIDI compatibility. See MIDI-CONTROLS.md for the complete supported list.

Incoming changes to these five parameters update the front-panel values. A knob
continues from the received value rather than its previous local value. Audio
publishes a packed value/generation word per parameter; the UI consumes the
latest word without clearing shared dirty flags. Controller changes remain on
the audio-owned synth; no LCD work is done in the audio callback. The new UI
state costs 848 bytes of normal RAM. Voice DSP and audio buffer are unchanged.

Validation: 640 CC/direct-parameter state and stereo-render comparisons,
input bounds, original CC5/10/91 number checks, and scripted MIDI-to-display-to-
knob continuity pass. Full suite passes, and all 29 previous WAV recordings
remain byte-identical. Target build, SRAM/cache placement and FWSC checks pass.

TEST32 boots in the 240 MHz browser emulator; performance navigation, note
activation and panic work. Firmware counters displayed zero xruns/deadlines.
The browser again reported insufficient real-time performance. MIDI CC feedback
was verified in the host tests, not through an external MIDI device in the
browser. Hardware CPU, external MIDI timing and sound quality remain untested.

## TEST31 patch volume

HOME -> FX eight times -> K2 selects PATCH VOLUME, 0–127. It replaces the
amplifier-response page's duplicate attack shortcut; amp attack remains on ENV.
Default 100 is unity and preserves previous audio. Source curve points:
0 = silence, 32 = 0.07357, 64 = 0.3518, 100 = 1, 127 = 2.1863 amplitude gain.
Higher settings can clip; physical MASTER remains the final output control.

The original patch-volume lookup curve is generated from the captured source.
It multiplies amplifier velocity and MIDI CC7/CC11 in cached per-voice targets.
Held/releasing voices use the native 32-sample amplitude ramp; new notes start
at the current gain. Changing patch volume does not reset oscillator/LFO phase
or envelopes. There is no new per-sample lookup or multiplication. Panic retains
the setting; reboot restores 100. Persistent patches are still not implemented.

The native output calibration remains .16/.631 rather than the source's
.2137876386687797 normalization. This ports the patch-volume curve, not absolute
output-level parity or the source's combined stereo gain interpolation.

Validation: 5,632 curve/controller-product/ramp values pass (maximum error
1.70e-6), plus phase/envelope continuity, mute and restoration assertions. Full
host suite passes; all 28 prior WAV recordings remain byte-identical at 100.
Scripted UI checks four voices, values 64/0/127/100, expression and panic.
Target build, executable/cache placement and FWSC validation pass.

Browser emulator at 240 MHz: TEST31 boots, PATCH VOLUME displays 100, dragging
K2 changes it to 93, note input activates a voice, and panic clears it. Firmware
counters displayed zero xruns/deadlines. The browser also reported insufficient
real-time audio performance, so neither audio quality nor physical CPU is
validated by this check. TEST31 hardware testing remains outstanding.

## TEST30 MIDI volume and expression

MIDI CC7 controls channel volume and CC11 controls expression. Each uses the
original squared curve (value / 127)^2; the gains multiply. Both initialize to
127/unity, preserving prior audio. At 64 either control gives approximately
25.4% amplitude; setting both to 64 gives approximately 6.45%. Zero mutes.

Held and releasing voices follow changes through the existing 32-sample native
amplitude ramp without restarting envelopes, oscillators or LFOs. New notes
start at the current controller gain. Panic clears voices but retains controller
levels; reboot restores unity. Input remains the existing single-part omni MIDI
path, so channels share these controls. No UI page or patch-volume mapping is
added. Physical MASTER still controls final output independently.

Controller arithmetic runs only on MIDI changes and voice parameter updates;
no extra per-sample multiplication or exponential was added. This retains the
native separate amplitude/pan smoothing described under TEST27, rather than
claiming identical source transient interpolation for simultaneous changes.

Checks passed: all 16,384 controller combinations compared with the original
engine (maximum gain error 1.52e-7), four-voice gain ramps (maximum error 3.09e-7),
held/releasing-note mute, state continuity, clamping, panic retention and reset.
All 27 previous WAV recordings are byte-identical at default controller levels.
The scripted MIDI path checks four voices, controller changes and panic. Full
host tests, target build, SRAM placement and FWSC validation pass.

Final TEST30 boots in the 240 MHz browser emulator with the correct display
label; performance navigation, note activation and panic pass, with firmware
xrun/deadline counters at zero during the short check. An earlier emulator run
reported browser audio-buffer dropouts even with its larger buffer. Emulator
results do not establish physical CPU, sound quality or CC7/11 MIDI input there;
the controller behavior was checked in the host/source tests. Hardware untested.

## TEST29 polyphonic portamento

HOME -> FX nine times -> SEL -> K4 selects PORTAMENTO (0–127).
0 is OFF and preserves previous audio. 64 is approximately 0.103 seconds,
96 is 0.741 seconds and 127 is 5 seconds. These are exponential time constants,
not fixed times to reach the destination note.

New voices glide from the previous MIDI note target. The first note starts at
its own pitch. Four-voice polyphony is retained; mono/legato key modes are not
added. Panic retains the last-note reference, matching the source. Disabling
portamento snaps active voices to their targets without restarting envelopes.
Oscillator pitch, filter key tracking and note-following saturation follow the
glide; LFO key tracking still uses the MIDI note. Oscillator-2 key tracking is
still available under HOME -> FX twice -> SEL -> K4.

Original exponential control coefficients are generated for 44.1 kHz. Pitch
updates every 32 samples use compensated float32 accumulation so the slowest
glide does not stall near its target. No per-sample exponential is added.

Validation: 256 source timing/coefficient values and 1,200,000 pitch, oscillator,
filter and saturation control values pass (maximum control error 5.09e-6).
First-note, previous-note, panic, disable, envelope continuity and slow-glide
completion checks pass. All 26 prior WAV regressions are byte-identical with
glide OFF. Full host tests, target build, SRAM placement and FWSC checks pass.
The wave bank is unchanged. Four voices/core 0/256 frames remain unchanged.

Browser emulator smoke test at 240 MHz: TEST29 boots, GLO opens performance,
a keyboard note activates a voice, and PLAY/panic clears voices. Firmware
counters displayed zero xruns/deadlines in this short test. The emulator's
status also reported five short audio quanta; these are a separate emulator
indicator. No listening or active four-voice glide performance claim is made.
Emulator CPU numbers are not physical FM-1 measurements.

TEST28 user feedback: no CPU change; exact percentages, error counters and
listening results were not restated. TEST29 physical hardware is untested.

## TEST28 transpose and pitch-bend controls

HOME -> FX nine times opens PITCH / BEND. K1 is global transpose, K2 is bend-up
range, K3 is bend-down range and K4 selects LINEAR or CURVED bend response.
Transpose and bend ranges display signed semitones (raw source value minus 64).
Defaults remain transpose +0, bend up +2, bend down -2, LINEAR.
In TEST28, SEL exposed oscillator 1/2 tuning and key tracking. TEST29 replaces
the duplicate oscillator-2 keytrack shortcut with PORTAMENTO.

The source ranges are retained: -64 to +63 semitones for each signed control.
Bend ranges can deliberately reverse direction. CURVED applies signed-square
bend (b * abs(b)): at half wheel travel its pitch offset is one quarter of the
selected range. Transpose is added separately. MIDI maximum positive bend
is 16383/8192 - 1, just below +1, retaining the existing MIDI normalization.

The pitch offset is cached when active voices update. It reaches all three
oscillators, their keyboard tracking, filter keyboard tracking and note-following
saturation. LFO key tracking continues to use the original MIDI note, matching
the source. Changing transpose or ranges updates held notes without restarting
their envelopes. That TEST28 change did not add portamento; TEST29 adds it above.

Validation checks 45,360 source control values across notes, transpose values,
signed ranges, both response curves, nine MIDI bend positions and four
oscillator-3 mode families. Maximum control error is 8.42e-6; note-following
rate-period relative error is 3.15e-7. Held-note checks cover no envelope
retrigger and unchanged LFO keytrack. Stress and render-block invariance include
the new controls. All twenty-five previous recordings remain byte-identical
at defaults. The wave bank and flash layout are unchanged. Subsequent TEST28 feedback
reported no CPU change.

TEST27 feedback: no CPU change. Exact percentages, error counters and amplifier
velocity listening results were not restated. TEST22 remains the latest explicit
zero-xrun/deadline confirmation.

## TEST27 amplifier velocity response

HOME -> FX eight times opens AMPLIFIER RESPONSE. K1 is AMP VELOCITY. The
native LINEAR setting (128) remains the default and preserves previous audio.
Values 0–127 select the original source T amplifier-velocity curve:

- 64: amplitude does not depend on note velocity.
- 65–127: softer notes are attenuated, with greater sensitivity at higher values.
- 0–63: inverse response; harder notes are attenuated.
- LINEAR (128): previous native noteVelocity / 127 response.

This changes amplifier level only. The dedicated filter, oscillator, FM and
pan velocity destinations continue to receive the original note velocity.
The existing output normalization remains; source patch/global volume is not
included in this change. The page also exposes amp attack, decay, sustain,
release, slope, pan and oscillator volume for convenient comparisons.

The curve is calculated at note start and parameter updates and cached per
voice. A native 32-sample level ramp handles changes on held notes; new notes
start at their calculated gain. This level ramp is separate from pan smoothing.
When amplitude and pan targets change simultaneously, their independently
smoothed product is not the source's combined stereo-gain interpolation.
Static targets and the original velocity curve are source-matched.

Validation covers 16,512 curve values (all source settings/velocities plus
LINEAR), 61,440 stereo audio samples across all filter routes, and 25,800
level-ramp samples. Maximum curve error is 8.18e-8; audio error is 9.32e-8;
native ramp error is 9.54e-7. Stress and render-block invariance include source
velocity settings. All twenty-four prior recordings remain byte-identical in
LINEAR mode. Wave bank and flash layout are unchanged. TEST27 later feedback reported unchanged CPU; other results were not restated.

TEST26 feedback: no CPU change; exact percentages, error counters and listening
results were not restated. TEST22 remains the latest explicit zero-xrun and
zero-deadline report.

## TEST26 oscillator and pan velocity

HOME -> FX seven times opens OSC / PAN VELOCITY. The upper bank contains
VEL SHAPE 1, VEL SHAPE 2, VEL WIDTH and VEL PAN. Each defaults to neutral 64.
SEL exposes oscillator shapes 1/2 and widths 1/2. Higher sensitivity values
add positive modulation on harder notes; lower values add negative modulation.

These four destinations use the original 2 * (amount - 64) * noteVelocity
formula. Shape velocity is added before clipping and combines with LFO 2 shape
modulation. Shared pulse-width velocity combines with LFO 1 and LFO 3 width
modulation using the original -63.5 to 127 limits. Oscillator 3 SLAVE follows
oscillator 2's shape; its pulse mode follows oscillator 2's modulated width.
Wave modes retain their pure-wave shape.

Pan velocity combines with LFO 2 panorama and split width, using the existing
per-voice 32-sample pan smoothing. New notes start at their calculated pan;
turning the destinations neutral returns smoothly to the shared static pan.
Velocity offsets are cached per voice on note start and parameter updates.
The native output-velocity curve remains unchanged.

Validation compares 4,177,920 oscillator-control, pan-slew and stereo-routing
values with the captured source. Coverage includes every sensitivity value,
five velocities, all four filter routes, all four oscillator-3 mode families,
and simultaneous LFO shape/width/pan modulation. Maximum error is 9.61e-7
(limit 3e-5). The existing oscillator audio comparisons remain in the suite.
Stress and render-block invariance checks include the four new destinations.
All twenty-three prior recordings remain byte-identical at neutral settings.
The wavetable bank and flash layout are unchanged. TEST26 later feedback reported unchanged CPU; other results were not restated.

TEST25 feedback: no CPU change. Exact percentages, error counters and
soft/hard-note listening results were not restated. TEST24's punch behavior
was confirmed; TEST22 is the latest explicit zero-xrun/deadline report.

## TEST25 filter velocity sensitivity

HOME -> FX six times opens FILTER VELOCITY. The upper bank contains VEL ENV 1,
VEL ENV 2, VEL RES 1 and VEL RES 2. All default to neutral 64. Values above
64 add envelope amount or resonance with increasing note velocity; values below
64 subtract them. SEL exposes the existing envelope amounts and resonances for
convenient comparison. Existing filter-envelope polarity still applies.

This ports the source's four dedicated velocity destinations. Each uses
2 * (amount - 64) * (note velocity / 127). Envelope offsets enter the original
nonlinear amount curve before its polarity and envelope-level multiplication.
The source's special 127-to-128 endpoint and fractional amount interpolation
are preserved. Resonance offsets combine with LFO resonance and clamp to
0–127. Linked cutoff, key tracking, LFO cutoff and filter routing remain intact.

The velocity-dependent envelope amounts and resonance offsets are cached per
voice at note start and parameter changes. They do not require recalculating
the amount curve for every sample or control tick. This does not change the
existing native linear output-velocity response; the source amp-velocity curve
and other velocity destinations remain to be ported.

Validation covers 2,049 fractional envelope values, including the endpoint
jump, plus 1,013,760 source control/audio values across all routes, both filters,
128 sensitivity values, five MIDI velocities, three envelope levels and LFO
resonance. Maximum error is 9.38e-6 (limit 8e-5). Stress and render-block
invariance checks include all four new destinations. All twenty-two previous
recordings remain byte-identical with sensitivity controls neutral. The bank
and flash layout are unchanged. TEST25 later feedback reported unchanged CPU; other results were not restated.

TEST24 feedback: no CPU change and punch acting as expected. Exact percentages
and error counts were not restated. TEST22 is the latest explicit confirmation
of zero xruns/deadline misses.

## TEST24 note-start punch

HOME -> FX five times -> SEL -> K4 is PUNCH (0–127). It replaces the duplicate
filter-2 resonance shortcut on this bank; filter-2 resonance remains on the
FILTER page's SEL bank. PUNCH defaults to zero, a native true OFF setting.
Values 1–127 use the original source intensity curve and twelve-sample impulse.

Each new note starts the impulse on the oscillator-1/filter-1 input side,
before LFO filter gain, filtering and saturation. In serial/parallel routes it
joins the normal summed input; in SPLIT it enters only the filter-1 branch.
The amplitude follows OSC VOLUME, including its original small nonzero floor
at value zero. Control changes, pitch bend and note release do not restart it.
The filter and amp envelope shape its audibility; compare short repeated notes.

The source retains a tiny punch at intensity zero (approximately 0.00009988
before oscillator-volume scaling). Native zero explicitly bypasses that residual
to preserve previous recordings. All active intensity values and the impulse
shape are source-derived. The impulse lasts twelve samples, not twelve control
blocks, and is independent of the note's duration.

Validation checks all 128 source intensity entries and 307,200 stereo values
across filter routes, oscillator-volume ranges and saturation families. Active
punch audio differs from the source by at most 9.54e-7 (limit 5e-5); zero uses
the documented native bypass in both sides of the comparison. Lifecycle checks
cover new-note reset and no retrigger on parameter changes, bend or release.
Stress and render-block invariance tests include punch. All twenty-one earlier
recordings remain byte-identical with punch OFF. The wave bank is unchanged.

TEST23 feedback: no CPU change; exact percentages, error counts and listening
results were not restated. TEST22 explicitly confirmed zero xruns/deadline
misses and unchanged CPU. TEST24 later feedback confirmed unchanged CPU and expected punch behavior.

## TEST23 complete oscillator-volume range

HOME -> FX five times -> K2 is now OSC VOLUME (0–127), replacing SAT DRIVE.
The default remains 64. Values 0–63 use the original oscillator-volume curve
to reduce oscillators 1, 2, 3 and the sub before the filters. Values 64–127
keep their gain at unity and control the selected saturation mode, exactly as
in TEST22. K1 still chooses the saturation curve. OFF bypasses saturation but
continues to respect the lower oscillator-volume range.

Noise and ring modulation keep their separate levels, matching the source.
OSC VOLUME is therefore not a master output control: noise or ring can remain
audible at zero. FM continues to use the raw oscillator signal. Oscillator 3's
slave, saw, pulse and independent wavetable modes all receive the same volume
curve. The original 64–127 behavior and the lossless wave bank are unchanged.

Validation checks all 128 curve entries and 1,048,576 stereo audio values
against the captured source across all four filter routes and oscillator-3
mode families, with sub, ring and SOFT saturation enabled. Maximum audio error
is 8.19e-6 (limit 5e-5). Separate checks confirm noise/ring samples and noise
state remain unchanged across the whole volume range. Stress and render-block
invariance tests now exercise values below 64 under ASan/UBSan. All twenty
previous recordings remain byte-identical at their previous settings.

TEST22 hardware feedback: no CPU change, zero xruns and zero deadline misses.
Exact CPU percentages were not restated. TEST23 later feedback reported no CPU change; other results were not restated.

## TEST22 filter-1 saturation

HOME -> FX five times opens FILTER SATURATION. K1 selects the curve; K2 sets
SAT DRIVE from 64 to 127. K3/K4 retain routing/filter balance. SEL exposes both
filters' cutoff and resonance. Saturation defaults to OFF and drive to 64.

All 14 active source modes are ported: Light, Soft, Middle, Hard, Digital,
Shaper, Rectifier, Bit Reducer, Rate Reducer, Rate + Follow, Low Pass,
Low + Follow, High Pass and High + Follow. The eight transfer curves use the
original five drive anchors and seventeen-point tables, including source
extrapolation outside the central range. Rate and filter modes retain their
source frequency curves, note following and state across parameter changes.
They process filter 1's output, before filter 2 in the serial routes and on the
filter-1 branch in parallel/split routes. This is not a master distortion.

TEST22 SAT DRIVE mapped the original oscillator-volume control's 64–127
saturation range. TEST23 completes the 0–63 range with OSC VOLUME above.
Coefficient changes happen at control updates. The sample processing kernel
runs in executable RAM. Four voices and the existing buffer are unchanged.

Source comparisons pass for all modes and drive values, three note-follow
pitches and all four filter routes. Maximum component error is 9.51e-7;
routed audio error is 3.33e-6. Rate periods are checked separately (relative
error below 8.49e-8), then snapped to shared 1/16-sample periods in the test
only to isolate reducer arithmetic from float32/float64 threshold drift.
Production code uses the unsnapped source rate formula.

All seventeen prior feature recordings, the original demo and muted profiler
recording are byte-identical to TEST21 with saturation OFF. ASan/UBSan, all
host UI scripts, package CRC and RAM-placement checks pass. The compressed
wavetable bank and flash layout are unchanged. TEST22 later hardware feedback reported unchanged CPU and zero errors.

TEST21 feedback: no CPU change reported; exact values, error counters and
three-wave loading time were not restated. Last explicit confirmed result:
TEST19, 55% average / 62% peak, zero xruns/deadline misses.

## TEST21 independent oscillator-3 wavetable selection

HOME -> FX four times -> K1 now continues past OFF / SLAVE / SAW / PULSE to
WAVE 0 through WAVE 63. These are the original oscillator-3 modes 4–67.
Wave modes use oscillator 3's own volume, tuning and detune, while retaining its
oscillator-2 pitch modulation and source routing. The original saw/pulse/slave
modes remain available. The wave-mode oscillator is pure wavetable, as in the
source; it is not driven through the oscillator-2 shape morph.

The loader can keep three different decoded waves resident. Identical selections
share a ready cache. A new third wave is reconstructed only while muted, checked
against the original CRC and then published to audio. Notes received while
loading are discarded; play them again after LOADING WAVES - MUTED clears.
Rapid changes cancel stale selections. Selecting or leaving a wave mode can
clear held notes through the existing muted-loading handshake.

All 64 original wave tables and fourteen bands remain lossless float32 values.
The third cache's first four bands use 32,784 bytes of normal RAM; its other ten
bands use 81,960 bytes of pool RAM. The 16,384-byte reconstruction accumulator
moves to normal RAM. Existing caches are unchanged. Total decoded storage is
344,232 bytes. A read-only band-pointer directory supports the split; each audio
sample still reads one contiguous 2049-float band directly. Pointer selection
happens outside the sample loop. Decompression and software float64 remain
restricted to muted main-loop reconstruction.

The compressed bank bytes and wav-partition layout are unchanged, so no bank
rewrite is needed over TEST20. The target layout fits both RAM regions, and an
ELF placement check verifies every cache section, size and non-overlap. RAM
headroom is now limited; see the current target memory totals below.

Validation reconstructs all 1,835,904 source samples through the split third
cache, checks corruption/bounds rejection and verifies that rebuilding it leaves
the other two caches unchanged. Expanded oscillator-3 source comparisons cover
independent waves and audio in all routes. Host UI checks exercise three distinct
waves (0, 31, 63), cancellation, loading-time note rejection and shared selections.
All sixteen previous recordings are byte-identical for their existing modes.
TEST21 later feedback reported no CPU change; loading time was not restated.

TEST20 feedback: no CPU change reported; exact values, per-mode listening and
error counters were not restated. Last explicit confirmed result is TEST19:
55% average / 62% peak, zero xruns/deadline misses.

## TEST20 oscillator 3: slave, saw and pulse

From HOME press FX four times for OSCILLATOR 3:

| Bank | K1 | K2 | K3 | K4 |
|---|---|---|---|---|
| Top | Mode | Volume | Semitone tuning | Detune |
| SEL | Oscillator 2 shape | Oscillator 2 pulse width | Oscillator 2 tuning | Oscillator 2 detune |

Mode defaults to OFF. SLAVE follows oscillator 2's selected wave, shape and
pulse width, with detune mirrored around oscillator 2's base pitch. Its level
is the original 1.058 times oscillator 2's balance gain; the independent OSC3
volume/tune/detune controls do not affect slave mode. This produces the original
paired detuned sound, including shape modulation.

SAW and PULSE use oscillator 3's own +/-48-semitone tuning and original detune
curve. The raw detune setting runs 0–127 and is not cents; it spans 0 to about
-46.531 cents. Their level follows the original 1.673 * volume/127 curve, default
volume 64. Both modes retain oscillator 2's key tracking and pitch modulation;
PULSE also follows oscillator 2's pulse width and PWM. LFO/envelope destinations
that affect oscillator 2 consequently affect oscillator 3 as in the source.
Oscillator 3 does not receive oscillator 2's FM or hard-sync processing.

Oscillator 3 joins the oscillator-2 side before the filters, including filter 2
in split mode. Ring modulation still uses only oscillators 1/2. It has its own
phase, BLEP state and pitch-band selection. SLAVE shares oscillator 2's cached
wave tables; SAW/PULSE need no new table cache. The audio-pool allocation and wave
bank are unchanged. Independent oscillator-3 wavetable selection remains absent.

OSC START also sets oscillator 3: fixed source angles use phase 1-value/128;
RANDOM draws an independent phase; the native RESET option starts all at zero.
The extra draw does not alter the existing oscillator 1/2 phase or noise streams.
Modes with zero output gain do not advance oscillator 3, matching source render
behavior. The native four-voice limit is retained; the web engine's automatic
voice-limit reduction for oscillator 3 is not used.

53,526 new source comparisons cover modes, tuning, detune, pitch/PWM/shape
modulation, gain and audio through all filter routes. Maximum error is below
6.5e-6. Phase checks, sanitizer stress, block-partition invariance and host UI
checks include oscillator 3. All fifteen previous recordings are byte-identical
with oscillator 3 OFF. TEST20 CPU and listening results are pending.

TEST19 is now hardware-confirmed: 55% average / 62% peak CPU, zero xruns and
deadline misses with LFO 3 enabled.

## TEST19 LFO 3 pitch, pulse width and fade-in

From HOME press LFO seven times:

| Bank | K1 | K2 | K3 | K4 |
|---|---|---|---|---|
| Top | LFO 3 rate | Shape | Amount | Destination |
| SEL | Fade-in | Rate key tracking | Clock division | Enable |

Destinations are PITCH 1, PITCH 1+2, PITCH 2, PWM 1, PWM 1+2 and PWM 2.
The six standard shapes are supported, with the source's fixed neutral contour.
LFO 3 has no source one-shot mode or note-trigger phase parameter: it begins
at its independently randomized phase. Native enable OFF pauses its generator
and removes its modulation. Defaults are OFF, amount 0, sine, rate 64, both
pitches, fade 0, clock OFF and key tracking 0.

The original LFO 3 pitch-depth curve differs from LFO 1: amount is unsigned
0–127, reaching 800 cents. Pulse-width depth uses the original linear amount.
LFO 3 combines with LFO 1 pitch/PWM and the oscillator-2 pitch envelope; each
oscillator's width is tracked separately. The pitch-band cache is reused.
Free rate includes the source's LFO 3 factor of 1.0058 below rate 127 and the
per-voice +/-2% deviation. Clocked rate bypasses key tracking/deviation, using
the existing shared BPM. Incoming MIDI clock is not connected.

Fade-in follows the original curve: a delay of one ninth of the selected total
time, then a linear rise to full modulation at that time. Maximum total time
is 42.41 seconds. Fade 64 reaches full depth after about 1.594 seconds; fade 0
has no delay. Timing runs from note start, including while native enable is OFF.
Start a new note to restart the fade. A bounded sample counter keeps fade timing
stable during long-held notes; it saturates only after every fade is complete.
The LFO 3 random stream is independent of noise, start phase and LFOs 1/2.

1,168,128 source comparisons cover all depths and destinations, all fade settings
and live LFO 3/LFO 1 combinations with free and clocked rates. Largest error is
below 3.91e-5 (live LFO values); depth routing errors are below 4.42e-8. Sanitizer
stress and block-partition checks exercise all three LFOs together. All fourteen
previous recordings are byte-identical with LFO 3 OFF; wave-bank bytes are
unchanged. TEST19 needs
hardware listening and CPU checks. Global/mono mode and wavetable LFO shapes
remain unimplemented.

TEST18 feedback: no CPU change reported; stereo listening and error counters
were not explicitly restated. TEST15–17 also reported no CPU change. Last explicit
measurements remain TEST14: 56% average / 63% peak, zero xruns/deadline misses.

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
| LFO seventh press | LFO 3 rate, shape, amount, destination | Fade-in, key tracking, clock, enable |
| FX | Amp slope, filter slope, pan, split width | Sub level, sub shape, ring level, oscillator start phase |
| FX again | Noise level, noise colour, pulse widths 1/2 | Oscillator tuning 1/2, oscillator key tracking 1/2 |
| FX third press | FM amount, source, filter-envelope amount, velocity amount | Oscillator 2 pitch-envelope amount, filter attack, decay, sustain |
| FX fourth press | Oscillator 3 mode, volume, tuning, detune | Oscillator 2 shape, width, tuning, detune |
| FX fifth press | Saturation curve, oscillator volume, routing, filter balance | Filter 1 cutoff/resonance, filter 2 cutoff, punch |
| FX sixth press | Velocity envelope 1/2, velocity resonance 1/2 | Envelope amounts 1/2, resonances 1/2 |
| FX seventh press | Velocity shape 1/2, velocity width, velocity pan | Oscillator shapes 1/2, widths 1/2 |
| FX eighth press | Amp velocity, attack, decay, sustain | Amp release, slope, pan, oscillator volume |
| FX ninth press | Transpose, bend up, bend down, bend curve | Oscillator tuning 1/2, key tracking 1/2 |

To hear the new waves, set Shape 1 and Shape 2 to 0. Shape 64 is saw and shape
127 is pulse; changing the wave number is inaudible in these pure saw/pulse modes.
For pulse-width testing use shape 127 and intermediate widths, such as 48/80.
Tune range is +/-48 semitones. Oscillator key tracking defaults to 100%, centred
on MIDI note 60; 0% holds a fixed pitch. Sub pitch follows oscillator 1.

GLO opens performance; K2 resets counters. Reset AFTER loading before comparing
playback CPU. GLO again opens the unchanged fixed, muted six-stage profiler;
K1 selects its stage. PLAY panics; OCT-/OCT+ changes the onboard keyboard octave.
MASTER sets output level. PRESET/SAVE/ARP/SEQ/REC are unassigned. ENV again
selects its filter bank. LFO cycles filter modulation, LFO 1 oscillators, LFO 1 filters, LFO 1 timing, LFO 2 filters, LFO 2 oscillators and LFO 3; FX cycles its nine pages.

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

Target application: 246,452 / 581,564 bytes. Normal data+BSS: 83,224 / 98,304 bytes.
Audio pool: 327,836 / 344,064 bytes. Executable SRAM: 15,292 / 24,576 bytes,
including 12,324 bytes of synth/profile kernels. Host synth struct: 3,472 bytes.
FWSC: 612,324 bytes, identity FM-1_15002800, version 0.28.0 / TEST28.

## Building and reproducing checks

```
node tools/generate_tables.cjs
node tools/generate_saturation.cjs
node tools/generate_wave_bank.cjs
python3 tools/pack_wave_bank.py
./tests/run.sh
./build-host.sh
mkdir -p build/test38-lfo-waves build/test37-hold-mode build/test36-key-modes build/test35-midi-lfo build/test34-midi-waves build/test33-midi-synth build/test32-midi-parameters build/test31-patch-volume build/test30-midi-volume build/test29-portamento build/test28-pitch-controls build/test27-amp-velocity build/test26-osc-velocity build/test25-filter-velocity build/test24-punch build/test23-osc-volume build/test22-saturation
app/build/host/virooos_host tests/test38-lfo-waves.txt build/test38-lfo-waves
app/build/host/virooos_host tests/test37-hold-mode.txt build/test37-hold-mode
app/build/host/virooos_host tests/test36-key-modes.txt build/test36-key-modes
app/build/host/virooos_host tests/test35-midi-lfo.txt build/test35-midi-lfo
app/build/host/virooos_host tests/test34-midi-waves.txt build/test34-midi-waves
python3 tests/check-midi-waves.py
app/build/host/virooos_host tests/test33-midi-synth.txt build/test33-midi-synth
app/build/host/virooos_host tests/test32-midi-parameters.txt build/test32-midi-parameters
app/build/host/virooos_host tests/test31-patch-volume.txt build/test31-patch-volume
app/build/host/virooos_host tests/test30-midi-volume.txt build/test30-midi-volume
app/build/host/virooos_host tests/test29-portamento.txt build/test29-portamento
app/build/host/virooos_host tests/test28-pitch-controls.txt build/test28-pitch-controls
app/build/host/virooos_host tests/test27-amp-velocity.txt build/test27-amp-velocity
app/build/host/virooos_host tests/test26-osc-velocity.txt build/test26-osc-velocity
app/build/host/virooos_host tests/test25-filter-velocity.txt build/test25-filter-velocity
app/build/host/virooos_host tests/test24-punch.txt build/test24-punch
app/build/host/virooos_host tests/test23-osc-volume.txt build/test23-osc-volume
app/build/host/virooos_host tests/test22-saturation.txt build/test22-saturation
app/build/host/virooos_host tests/test21-three-waves.txt build/test21-three-waves
app/build/host/virooos_host tests/test20-osc3.txt build/test20-osc3
app/build/host/virooos_host tests/test19-lfo3.txt build/test19-lfo3
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

This is not a complete Virus C/web-app port. External/auxiliary FM inputs, remaining LFO functions,
modulation matrix, arpeggiator, unison, SysEx patches and preset persistence remain.
Effects WASM is captured reference material, not linked. The wrapper defaults to
linear velocity, offers the source amplifier-velocity and patch-volume curves, and retains native output calibration. Start phase is now selectable. Immediate voice
stealing/parameter changes can click; high combined levels/resonance can clip.

Original public deployment assets are in `reference/`, captured 2026-11-12;
URLs/hashes are in `reference/manifest.json`. No original source licence was
established; this checkpoint asserts no redistribution licence for those assets.
Platform copyright/licence notices are preserved. The platform came from the
user's FM1-VStation source; that synth is not used. The wTABLES load-time/cache
architecture informed this loader, but its PCM16 bank codec is not reused.
