#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
: "${1:?Pass the absolute path to the wabt Node module}"
cc -std=c99 -O2 -ffp-contract=off -Iengine tests/eq.c engine/eq.c -lm -o build/eq
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -Iengine tests/eq.c engine/eq.c -lm -o build/eq-sanitized
node tests/eq.cjs "$1" | tee build/eq-comparison.json
ASAN_OPTIONS=detect_leaks=0 build/eq-sanitized < build/eq-input.txt > /dev/null
EQ_LIVE=1 node tests/eq.cjs "$1" | tee build/eq-live.json
ASAN_OPTIONS=detect_leaks=0 build/eq-sanitized < build/eq-input.txt > /dev/null
EQ_STRESS=1 node tests/eq.cjs "$1" | tee build/eq-stress.json
ASAN_OPTIONS=detect_leaks=0 build/eq-sanitized < build/eq-input.txt > /dev/null
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_POST_FX -Iengine -Ibase/include tests/eq-integration.c engine/virooos.c engine/lfo.c engine/saturation.c engine/distortion.c engine/eq.c -lm -o build/eq-integration
ASAN_OPTIONS=detect_leaks=0 build/eq-integration | tee build/eq-integration.txt
