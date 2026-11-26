#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build validation
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_MATRIX -Iengine -Ibase/include tests/matrix-pwm.c engine/lfo.c engine/saturation.c engine/matrix.c -lm -o build/matrix-pwm
ASAN_OPTIONS=detect_leaks=0 node tests/matrix-pwm.cjs | tee validation/test57-matrix-pwm.json
