#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off tests/band-rebuild.c experimental/band_rebuild.c -lm -o build/band-rebuild
ASAN_OPTIONS=detect_leaks=0 build/band-rebuild | tee validation/band-rebuild-prototype1.json
