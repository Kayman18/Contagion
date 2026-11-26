#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_MATRIX -Iengine -Ibase/include tests/matrix-detune.c engine/lfo.c engine/saturation.c engine/matrix.c -lm -o build/matrix-detune
ASAN_OPTIONS=detect_leaks=0 node tests/matrix-detune.cjs | tee validation/test69-matrix-detune.json
