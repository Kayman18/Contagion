#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p presets
model="${VIRUS_MODEL:-ALL}"
voices="${VOICES:-8}"
case "$model" in
  A) release=84 ;;
  B) release=85 ;;
  C) release=86 ;;
  ALL) release=90 ;;
  *) echo "VIRUS_MODEL must be A, B, C or ALL" >&2; exit 1 ;;
esac
case "$voices" in
  8) ;;
  *) echo "Contagion releases use VOICES=8" >&2; exit 1 ;;
esac
banks="${VIRUS_BANK_DIR:-reference/factory-sysex/Virus-$model}"
if [[ "$model" == ALL ]]; then banks="${VIRUS_BANK_DIR:-reference/factory-sysex}"; fi
node tools/generate_presets.cjs "$model" "$banks"
node tools/generate-matrix.cjs
node tools/generate_preset_status.cjs
python3 tools/pack_factory.py "$model"
python3 tools/pack_eq.py
python3 tools/make_wave_cache.py
printf '/* Set by build-firmware.sh. */\n#define VC_VOICES_BUILD %s\n' "$voices" > engine/voice_count.h
export AC79_SDK="${AC79_SDK:-$PWD/build-deps/sdk}"
python3 base/tools/build.py app --release "0.$release.$voices" --display-version "CONTAGION 1.1" "$@"
