#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -pthread tests/band-handoff.c experimental/band_handoff.c experimental/band_cache.c experimental/band_rebuild.c -lm -o build/band-handoff
ASAN_OPTIONS=detect_leaks=0 build/band-handoff | tee validation/band-handoff-prototype3.json
