#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_POST_FX -DVC_DYNAMIC_WAVES -Iengine -Ibase/include tests/presets.c engine/presets.c engine/virooos.c engine/wave_cache.c engine/lfo.c engine/saturation.c engine/distortion.c engine/eq.c -lm -o build/presets-test
ASAN_OPTIONS=detect_leaks=0 build/presets-test > build/preset-params.bin
node tests/presets.cjs | tee build/preset-validation.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -Iengine -Ibase/include tests/lfo3-global.c engine/lfo.c engine/saturation.c -lm -o build/lfo3-global
ASAN_OPTIONS=detect_leaks=0 node tests/lfo3-global.cjs | tee build/lfo3-global-comparison.json
