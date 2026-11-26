#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
# Four-voice stealing/client-limit scenarios: built at VC_VOICES=4 on purpose
# (the logic is voice-count independent; tests/twelve-voices.txt covers 12).
mkdir -p build validation
cc -DVC_VOICES=4 -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -Iengine -Ibase/include tests/unison.c engine/lfo.c engine/saturation.c -lm -o build/unison-test
ASAN_OPTIONS=detect_leaks=0 build/unison-test | tee validation/test60-unison.txt
