#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p presets
model="${VIRUS_MODEL:-C}"
voices="${VOICES:-8}"
case "$model" in
  A) release=81 ;;
  B) release=82 ;;
  C) release=83 ;;
  *) echo "VIRUS_MODEL must be A, B or C" >&2; exit 1 ;;
esac
case "$voices" in
  8) ;;
  *) echo "Contagion releases use VOICES=8" >&2; exit 1 ;;
esac
banks="${VIRUS_BANK_DIR:-reference/factory-sysex/Virus-$model}"
node tools/generate_presets.cjs "$model" "$banks"
node tools/generate-matrix.cjs
node tools/generate_preset_status.cjs
printf '/* Set by build-virus-banks.sh. */\n#define VC_VOICES_BUILD %s\n' "$voices" > engine/voice_count.h
export AC79_SDK="${AC79_SDK:-$PWD/build-deps/sdk}"
python3 base/tools/build.py app --release "0.$release.$voices" --display-version "CONTAGION ${voices}V" "$@"
