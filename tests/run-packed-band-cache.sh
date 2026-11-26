#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off tests/packed-band-cache.c experimental/band_bank.c experimental/band_handoff.c experimental/band_cache.c experimental/band_rebuild.c -lm -o build/packed-band-cache
ASAN_OPTIONS=detect_leaks=0 build/packed-band-cache | tee validation/packed-band-cache-prototype5.json
