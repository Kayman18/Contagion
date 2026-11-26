#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_MATRIX -Iengine -Ibase/include tests/matrix-balance.c engine/lfo.c engine/saturation.c engine/matrix.c -lm -o build/matrix-balance
ASAN_OPTIONS=detect_leaks=0 node tests/matrix-balance.cjs | tee validation/test72-matrix-balance.json
