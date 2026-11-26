#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
cc -DPLAT_HOST -DVC_MATRIX -std=c11 -O1 -g -fsanitize=address,undefined -ffp-contract=off tests/band-preload.c experimental/band_bank.c experimental/band_rebuild.c experimental/band_cache.c experimental/band_handoff.c experimental/band_voice.c engine/matrix.c -lm -o build/band-preload
ASAN_OPTIONS=detect_leaks=0 build/band-preload | tee validation/band-preload-prototype8.json
