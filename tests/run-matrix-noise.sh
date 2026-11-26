#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_MATRIX -Iengine -Ibase/include tests/matrix-noise.c engine/lfo.c engine/saturation.c engine/matrix.c -lm -o build/matrix-noise
ASAN_OPTIONS=detect_leaks=0 node tests/matrix-noise.cjs | tee validation/test68-matrix-noise.json
