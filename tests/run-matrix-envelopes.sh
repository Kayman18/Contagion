#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_MATRIX -Iengine -Ibase/include tests/matrix-envelopes.c engine/lfo.c engine/saturation.c engine/matrix.c -lm -o build/matrix-envelopes
ASAN_OPTIONS=detect_leaks=0 node tests/matrix-envelopes.cjs | tee validation/test64-matrix-envelopes.json
