#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/components.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/components
node tests/compare.cjs | tee build/component-comparison.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Iengine -Ibase/include tests/engine.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/engine-test
ASAN_OPTIONS=detect_leaks=0 build/engine-test | tee build/engine-test.txt
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/filter-routing.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/filter-routing
node tests/filter-routing.cjs | tee build/filter-routing-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/sub.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/sub
node tests/sub.cjs | tee build/sub-comparison.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Iengine -Ibase/include tests/profile.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/profile-test
ASAN_OPTIONS=detect_leaks=0 build/profile-test | tee build/profile-test.txt

cc -std=c99 -O2 -ffp-contract=off -Iengine -Ibase/include tests/oscillator-optimization.c engine/lfo.c engine/saturation.c -lm -o build/oscillator-optimization
build/oscillator-optimization | tee build/oscillator-optimization.txt
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/noise.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/noise
node tests/noise.cjs | tee build/noise-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/osc-controls.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/osc-controls
node tests/osc-controls.cjs | tee build/osc-controls-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/waves.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/waves
node tests/waves.cjs | tee build/waves-comparison.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -Iengine tests/wave-cache.c engine/wave_cache.c -o build/wave-cache
ASAN_OPTIONS=detect_leaks=0 build/wave-cache > build/reconstructed-waves.f32
cmp data/source-waves.f32 build/reconstructed-waves.f32
printf 'PASS: all 1835904 reconstructed float32 samples byte-identical; bank corruption, truncation and bounds rejected\n' | tee build/wave-cache-validation.txt
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Wno-misleading-indentation -DPLAT_HOST -DVC_DYNAMIC_WAVES -Iengine -Ibase/include tests/waves.c engine/virooos.c engine/wave_cache.c engine/lfo.c engine/saturation.c -lm -o build/waves
ALL_WAVES=1 node tests/waves.cjs | tee build/all-waves-comparison.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_DYNAMIC_WAVES -Iengine -Ibase/include tests/engine.c engine/virooos.c engine/wave_cache.c engine/lfo.c engine/saturation.c -lm -o build/dynamic-engine-test
ASAN_OPTIONS=detect_leaks=0 build/dynamic-engine-test | tee build/dynamic-engine-test.txt
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/fm.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/fm
node tests/fm.cjs | tee build/fm-comparison.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -Iengine -Ibase/include tests/fm-noise.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/fm-noise
ASAN_OPTIONS=detect_leaks=0 build/fm-noise | tee build/fm-noise-validation.txt
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/fm-osc.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/fm-osc
node tests/fm-osc.cjs | tee build/fm-osc-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/pitch-env.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/pitch-env
node tests/pitch-env.cjs | tee build/pitch-env-comparison.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -Iengine -Ibase/include tests/start-phase.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/start-phase
ASAN_OPTIONS=detect_leaks=0 node tests/start-phase.cjs | tee build/start-phase-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/lfo.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/lfo
node tests/lfo.cjs | tee build/lfo-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/lfo-filter.c engine/lfo.c engine/saturation.c -lm -o build/lfo-filter
node tests/lfo-filter.cjs | tee build/lfo-filter-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/lfo-timing.c engine/lfo.c engine/saturation.c -lm -o build/lfo-timing
node tests/lfo-timing.cjs | tee build/lfo-timing-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/lfo2.c engine/lfo.c engine/saturation.c -lm -o build/lfo2
node tests/lfo2.cjs | tee build/lfo2-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/lfo2-osc.c engine/lfo.c engine/saturation.c -lm -o build/lfo2-osc
node tests/lfo2-osc.cjs | tee build/lfo2-osc-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/lfo2-pan.c engine/lfo.c engine/saturation.c -lm -o build/lfo2-pan
node tests/lfo2-pan.cjs | tee build/lfo2-pan-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/lfo3.c engine/lfo.c engine/saturation.c -lm -o build/lfo3
node tests/lfo3.cjs | tee build/lfo3-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/osc3.c engine/lfo.c engine/saturation.c -lm -o build/osc3
node tests/osc3.cjs | tee build/osc3-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/osc3-waves.c engine/lfo.c engine/saturation.c -lm -o build/osc3-waves
node tests/osc3-waves.cjs | tee build/osc3-waves-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/saturation.c engine/lfo.c engine/saturation.c -lm -o build/saturation
node tests/saturation.cjs | tee build/saturation-comparison.json

cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/osc-volume.c engine/lfo.c engine/saturation.c -lm -o build/osc-volume
node tests/osc-volume.cjs | tee build/osc-volume-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/punch.c engine/lfo.c engine/saturation.c -lm -o build/punch
node tests/punch.cjs | tee build/punch-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/filter-velocity.c engine/lfo.c engine/saturation.c -lm -o build/filter-velocity
node tests/filter-velocity.cjs | tee build/filter-velocity-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/osc-velocity.c engine/lfo.c engine/saturation.c -lm -o build/osc-velocity
node tests/osc-velocity.cjs | tee build/osc-velocity-comparison.json

cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/amp-velocity.c engine/lfo.c engine/saturation.c -lm -o build/amp-velocity
node tests/amp-velocity.cjs | tee build/amp-velocity-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/pitch-controls.c engine/lfo.c engine/saturation.c -lm -o build/pitch-controls
node tests/pitch-controls.cjs | tee build/pitch-controls-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/portamento.c engine/lfo.c engine/saturation.c -lm -o build/portamento
node tests/portamento.cjs | tee build/portamento-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/midi-volume.c engine/lfo.c engine/saturation.c -lm -o build/midi-volume
node tests/midi-volume.cjs | tee build/midi-volume-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/patch-volume.c engine/lfo.c engine/saturation.c -lm -o build/patch-volume
node tests/patch-volume.cjs | tee build/patch-volume-comparison.json
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/midi-parameters.c engine/lfo.c engine/saturation.c -lm -o build/midi-parameters
build/midi-parameters | tee build/midi-parameters-validation.txt
node tests/midi-map.cjs | tee build/midi-map-validation.txt

cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/key-modes.c engine/lfo.c engine/saturation.c -lm -o build/key-modes
node tests/key-modes.cjs | tee build/key-modes-comparison.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -Iengine -Ibase/include tests/key-modes.c engine/lfo.c engine/saturation.c -lm -o build/key-modes-sanitized
ASAN_OPTIONS=detect_leaks=0 build/key-modes-sanitized > /dev/null
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/hold-mode.c engine/lfo.c engine/saturation.c -lm -o build/hold-mode
node tests/hold-mode.cjs | tee build/hold-mode-comparison.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -Iengine -Ibase/include tests/hold-mode.c engine/lfo.c engine/saturation.c -lm -o build/hold-mode-sanitized
ASAN_OPTIONS=detect_leaks=0 build/hold-mode-sanitized > /dev/null
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/lfo-waves.c -lm -o build/lfo-waves
node tests/lfo-waves.cjs | tee build/lfo-waves-comparison.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -Iengine -Ibase/include tests/lfo-waves.c -lm -o build/lfo-waves-sanitized
ASAN_OPTIONS=detect_leaks=0 build/lfo-waves-sanitized > /dev/null

# Exercise the firmware's mapped extension path against the same source vectors.
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -DVC_DYNAMIC_WAVES -Iengine -Ibase/include tests/lfo-waves.c -lm -o build/lfo-waves
node tests/lfo-waves.cjs | tee build/lfo-mapped-waves-comparison.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DVC_DYNAMIC_WAVES -Iengine -Ibase/include tests/lfo-waves.c -lm -o build/lfo-waves-mapped-sanitized
ASAN_OPTIONS=detect_leaks=0 build/lfo-waves-mapped-sanitized > /dev/null
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/lfo-env-midi.c engine/lfo.c engine/saturation.c -lm -o build/lfo-env-midi
node tests/lfo-env-midi.cjs | tee build/lfo-env-midi-comparison.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -Iengine -Ibase/include tests/lfo-env-midi.c engine/lfo.c engine/saturation.c -lm -o build/lfo-env-midi-sanitized
ASAN_OPTIONS=detect_leaks=0 build/lfo-env-midi-sanitized > /dev/null
cc -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iengine -Ibase/include tests/lfo-global.c engine/lfo.c engine/saturation.c -lm -o build/lfo-global
node tests/lfo-global.cjs | tee build/lfo-global-comparison.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -Iengine -Ibase/include tests/lfo-global.c engine/lfo.c engine/saturation.c -lm -o build/lfo-global-sanitized
ASAN_OPTIONS=detect_leaks=0 build/lfo-global-sanitized > /dev/null

bash tests/run-active-lfo.sh
