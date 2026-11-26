#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build validation
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_MATRIX -Iengine -Ibase/include tests/matrix-shape.c engine/lfo.c engine/saturation.c engine/matrix.c -lm -o build/matrix-shape
ASAN_OPTIONS=detect_leaks=0 node tests/matrix-shape.cjs | tee validation/test56-matrix-shape.json
