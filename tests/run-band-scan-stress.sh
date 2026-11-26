#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
cc -std=c11 -O1 -g -fsanitize=address,undefined -ffp-contract=off tests/band-scan-stress.c experimental/band_bank.c experimental/band_handoff.c experimental/band_cache.c experimental/band_rebuild.c -lm -o build/band-scan-stress
ASAN_OPTIONS=detect_leaks=0 build/band-scan-stress | tee validation/band19-scan-stress.json
