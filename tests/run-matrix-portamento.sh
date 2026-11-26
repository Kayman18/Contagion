#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_MATRIX -Iengine -Ibase/include tests/matrix-portamento.c engine/lfo.c engine/saturation.c engine/matrix.c -lm -o build/matrix-portamento
ASAN_OPTIONS=detect_leaks=0 node tests/matrix-portamento.cjs | tee validation/test66-matrix-portamento.json
