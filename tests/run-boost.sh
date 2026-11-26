#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
: "${1:?Pass the absolute path to the wabt Node module}"
cc -std=c99 -O2 -ffp-contract=off -DPLAT_HOST -Iengine tests/boost.c engine/eq.c -lm -o build/boost
node tests/boost.cjs "$1" | tee validation/test61-boost.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -Iengine tests/boost.c engine/eq.c -lm -o build/boost-sanitized
ASAN_OPTIONS=detect_leaks=0 build/boost-sanitized < build/boost-input.txt > /dev/null
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_POST_FX -Iengine -Ibase/include tests/boost-integration.c engine/virooos.c engine/lfo.c engine/saturation.c engine/distortion.c engine/eq.c -lm -o build/boost-integration
ASAN_OPTIONS=detect_leaks=0 build/boost-integration | tee validation/test61-boost-integration.txt
