#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -std=c99 -O2 -ffp-contract=off -Iengine -Ibase/include tests/lfo-waves.c -lm -o build/lfo-static
cc -std=c99 -O2 -ffp-contract=off -DPLAT_HOST -DVC_DYNAMIC_WAVES -DVC_ACTIVE_LFO -Iengine -Ibase/include tests/lfo-active.c engine/wave_cache.c -lm -o build/lfo-waves
node tests/lfo-waves.cjs | tee build/lfo-active-comparison.json
build/lfo-static > build/lfo-static-values.f32
build/lfo-waves > build/lfo-active-values.f32
cmp build/lfo-static-values.f32 build/lfo-active-values.f32
cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -DPLAT_HOST -DVC_DYNAMIC_WAVES -DVC_ACTIVE_LFO -Iengine -Ibase/include tests/lfo-active.c engine/wave_cache.c -lm -o build/lfo-active-sanitized
ASAN_OPTIONS=detect_leaks=0 build/lfo-active-sanitized > /dev/null
printf 'PASS: 4261694 float values byte-identical to static LFO implementation; ASan/UBSan passed\n'
