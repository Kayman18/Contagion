#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
# Four-voice stealing/client-limit scenarios: built at VC_VOICES=4 on purpose
# (the logic is voice-count independent; tests/twelve-voices.txt covers 12).
cc -DVC_VOICES=4 -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off tests/band-cache.c experimental/band_cache.c experimental/band_rebuild.c -lm -o build/band-cache
ASAN_OPTIONS=detect_leaks=0 build/band-cache | tee validation/band-cache-prototype2.json
