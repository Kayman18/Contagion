#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
bash tests/run-matrix.sh
bash tests/run-matrix-filters.sh
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -DPLAT_HOST -DVC_MATRIX -DVC_POST_FX -DVC_DYNAMIC_WAVES -Iengine -Ibase/include tests/preset-render.c engine/presets.c engine/virooos.c engine/matrix.c engine/lfo.c engine/saturation.c engine/wave_cache.c engine/distortion.c engine/eq.c -lm -o build/preset-render50
ASAN_OPTIONS=detect_leaks=0 build/preset-render50 > validation/test50-preset-render.json
bash build-host.sh
mkdir -p build/host50-matrix build/host50-presets
app/build/host/virooos_host tests/test50-matrix.txt build/host50-matrix
app/build/host/virooos_host tests/test48-presets.txt build/host50-presets
