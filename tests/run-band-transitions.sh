#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_MATRIX -DVC_DYNAMIC_WAVES -DVC_BAND_CACHE -Iengine -Ibase/include tests/band-transitions.c engine/virooos.c engine/lfo.c engine/saturation.c engine/matrix.c experimental/band_voice.c experimental/band_bank.c experimental/band_handoff.c experimental/band_cache.c experimental/band_rebuild.c -lm -o build/band-transitions
ASAN_OPTIONS=detect_leaks=0 build/band-transitions | tee validation/band19-transitions.json
