#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
cc -std=c11 -O2 -ffp-contract=off -Itests/reference/band10-builder tests/band-speed.c tests/reference/band10-builder/band_rebuild.c -lm -o build/band-speed-before
cc -std=c11 -O2 -ffp-contract=off -Iexperimental tests/band-speed.c experimental/band_rebuild.c -lm -o build/band-speed-after
build/band-speed-before
build/band-speed-after
