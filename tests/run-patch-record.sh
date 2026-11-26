#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
cc -DPLAT_HOST -DVC_MATRIX -std=c11 -O1 -g -fsanitize=address,undefined -ffp-contract=off -Iengine -Ibase/include tests/patch-record.c experimental/patch_record.c engine/presets.c engine/matrix.c engine/virooos.c engine/lfo.c engine/saturation.c -lm -o build/patch-record
ASAN_OPTIONS=detect_leaks=0 build/patch-record | tee validation/patch-record-v1.json
