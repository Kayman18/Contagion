#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
# Four-voice stealing/client-limit scenarios: built at VC_VOICES=4 on purpose
# (the logic is voice-count independent; tests/twelve-voices.txt covers 12).
cc -DVC_VOICES=4 -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_MATRIX -DVC_DYNAMIC_WAVES -DVC_BAND_CACHE -Iengine -Ibase/include tests/band-voice.c engine/virooos.c engine/lfo.c engine/saturation.c engine/matrix.c experimental/band_voice.c experimental/band_bank.c experimental/band_handoff.c experimental/band_cache.c experimental/band_rebuild.c -lm -o build/band-voice
ASAN_OPTIONS=detect_leaks=0 build/band-voice | tee validation/band-voice-prototype6.json
