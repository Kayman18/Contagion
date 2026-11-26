#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build validation
node tools/generate-matrix.cjs
cc -std=c99 -Wall -Wextra -Werror -Wno-array-parameter -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -Iengine tests/matrix.c engine/matrix.c -o build/matrix-test
ASAN_OPTIONS=detect_leaks=0 node tests/matrix.cjs | tee validation/matrix-routing.json

ASAN_OPTIONS=detect_leaks=0 node tests/matrix.cjs --filters | tee validation/matrix-filter-reduced.json

ASAN_OPTIONS=detect_leaks=0 node tests/matrix.cjs --rounded | tee validation/matrix-slope-rounded.json
