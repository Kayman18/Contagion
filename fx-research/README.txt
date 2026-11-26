VIROOOS FM-1 EFFECTS MEMORY CHECKPOINT 01 — based on TEST43

RESULT: the full effects do not yet fit. Firmware is unchanged; TEST43 remains
our latest playable firmware. This checkpoint contains executable reference
research, not a new effects firmware or a claim of hardware performance.

CURRENT TARGET BUDGET (linked TEST43)
Normal data RAM 84,060 / 98,304 bytes: 14,244 free.
Buffer pool 327,836 / 344,064 bytes: 16,228 free.
Combined free data RAM 30,472 bytes (29.76 KiB), in separate regions.
Application flash free: 3,964 bytes. Effects code/tables also need flash space.
Decoded oscillator caches: 344,232 bytes (three waves, fourteen bands each).
Do not count stacks, reserved debug RAM, or free executable SRAM as a general
purpose audio-buffer allocation without separately proving a new memory map.

ORIGINAL BUFFER ALLOCATIONS
Chorus: two 1,024-sample double buffers = 16,384 bytes.
Delay: two 32,768-sample double buffers = 524,288 bytes.
Reverb: 32,768 doubles + 384 diffusion doubles = 265,216 bytes.
These exclude coefficients, scalar state and audio I/O buffers.
Delay A/B are the source's two delay buffers, not an assumed L/R stereo pair.
Original FX chain also includes distortion, analog boost, EQ and phaser.
Their smaller state/code requirements still need a native allocation audit.

WHY THE WASM MEMORY NUMBER IS NOT THE NATIVE REQUIREMENT
Original initialized linear memory is 18,743,296 bytes. The original stack
reservation alone is 16 MiB. This must not be quoted as the required FM-1 FX
RAM. Runtime, lookup data, allocator overhead and 4096-frame I/O buffers can
be removed, relocated or reduced in a native port. Delay-line state cannot.

VERIFIED STORAGE EXPERIMENT
Instrumented only f64 writes into the two delay and two reverb regions,
rounding their stored values through float32; all calculations and chorus
storage remained original double precision. Tested modes 1,2,3,4,5,10,25,
262,144 stereo frames per case (1,835,008 total), feedback 120, seeded noise
followed by decay, and delay-time changes. Every output sample matched exactly.
This is evidence for these cases, not an exhaustive proof for all settings
and not validation of converting the DSP calculations themselves to float32.
Chorus buffers are not exactly float32-representable in the tested cases:
maximum observed single-store rounding difference about 7.45e-9. That does
not establish audibility or bound accumulated feedback/output differences.

NATIVE MEMORY TARGETS (buffers only)
32-bit delay storage: 262,144 bytes.
32-bit reverb storage: 132,608 bytes.
32-bit chorus storage: 8,192 bytes (requires separate audio comparison).
Original double chorus storage: 16,384 bytes.
Delay/reverb are mutually exclusive in the source (modes 2–4 reverb, positive
remaining modes delay, zero bypass). A union can share their storage; clear
it on mode changes, matching the source's effect reset semantics.
Chorus + shared delay/reverb at 32-bit storage: 270,336 bytes / 264 KiB.
Minimum shortfall vs current free RAM: 239,864 bytes / 234.24 KiB.
Keeping double chorus raises that shortfall to 248,056 bytes / 242.24 KiB.
Additional scalar state, alignment, controls and safety margin are not included.
A 280 KiB FX reservation is a planning target, not an implemented guarantee.

NEXT IMPLEMENTATION GATES
1. Rework oscillator-table residency before enabling delay/reverb. Reclaiming
   the loading accumulator alone (16 KiB) or third oscillator cache alone
   (112 KiB) is insufficient for the full target.
2. Measure exact lossless cache-storage possibilities before choosing reduced
   table resolution/precision, shorter delay, or reduced polyphony. No such
   sound/range changes have been made by this checkpoint.
3. Reserve one delay/reverb arena with bounded compile/link checks, retaining
   stack and ISR headroom. Reuse existing audio blocks rather than 4096-frame
   WASM I/O arrays. Keep effect state per part, not per voice.
4. Port/test chorus and smaller effects, then delay/reverb with independent
   impulse, automation, stability and source comparisons. Validate float32 DSP
   math separately from the proven storage-only experiment.
5. Build and measure on the real unit; core splitting cannot solve RAM shortage.

FLASH / WAV PARTITION
plat_data is limited to 0x97000..0xE0000. Existing wave coefficients occupy
0x97000..0xAE000; extra LFO tables occupy 0xAE000..0xDB000 after sector padding.
Settings start at 0xDC000 and must be preserved. The existing first-boot lossless
loader avoids playback decompression but still needs decoded oscillator RAM.
Flash storage is suitable for static tables, not continuously rewritten delay
history. Do not extend writes into settings or reserved firmware regions.

REPRODUCE
Requires Node.js. From the checkpoint's virooos directory:
  node fx-research/audit.cjs
For the storage instrumentation test, install wabt in a separate local folder
(the test was run with the version recorded in wabt-package.json), then:
  node fx-research/test-buffer-storage.cjs /absolute/path/node_modules/wabt
The scripts check the captured original wasm hash / buffer structure.
Read audit.json and buffer-storage-comparison.json for measured results.
The reference wasm, processor and helper are included; no third-party runtime
or native firmware is bundled here. Original asset provenance is in reference/manifest.json.
