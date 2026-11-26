#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
: "${1:?Pass the absolute path to the wabt Node module}"
cc -std=c99 -O2 -ffp-contract=off -Iengine tests/distortion.c engine/distortion.c -lm -o build/distortion
node tests/distortion.cjs "$1" | tee build/distortion-comparison.json
node tests/distortion-boundaries.cjs "$1" | tee build/distortion-boundaries.json
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -Iengine tests/distortion.c engine/distortion.c -lm -o build/distortion-sanitized
ASAN_OPTIONS=detect_leaks=0 build/distortion-sanitized < build/distortion-input.txt > /dev/null
ASAN_OPTIONS=detect_leaks=0 build/distortion-sanitized < build/distortion-boundary-input.txt > /dev/null
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_POST_FX -Iengine -Ibase/include tests/distortion-integration.c engine/virooos.c engine/lfo.c engine/saturation.c engine/distortion.c engine/eq.c -lm -o build/distortion-integration
ASAN_OPTIONS=detect_leaks=0 build/distortion-integration | tee build/distortion-integration.txt
