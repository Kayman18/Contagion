#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
: "${JIELI_TOOLCHAIN:?Set JIELI_TOOLCHAIN}"
flags=(-target pi32v2 -isystem "$JIELI_TOOLCHAIN/pi32v2/include" -mcpu=r3 -mfprev1 -ffp-contract=off -std=c11 -O2 -fno-common -DVC_BAND_FM1_SINGLE_CORE -DVC_DYNAMIC_WAVES -DVC_MATRIX -DVC_POST_FX -DVC_ACTIVE_LFO -DVC_BAND_CACHE -Iengine -Ibase/include)
"$JIELI_TOOLCHAIN/pi32v2/bin/clang" "${flags[@]}" -c engine/virooos.c -o build/virooos-band-fm1.o
"$JIELI_TOOLCHAIN/pi32v2/bin/clang" "${flags[@]}" -c experimental/band_voice.c -o build/band-voice-fm1.o
"$JIELI_TOOLCHAIN/common/bin/objdump" -t build/virooos-band-fm1.o > validation/band-dsp-target-symbols.txt
"$JIELI_TOOLCHAIN/common/bin/objdump" -t build/band-voice-fm1.o > validation/band-voice-target-symbols.txt
python3 - <<'PY'
import pathlib,re,json
for name in ['band-dsp-target-symbols.txt','band-voice-target-symbols.txt']:
 text=pathlib.Path('validation',name).read_text()
 assert not re.search(r'__(?:add|mul|div|sub)df|__atomic|__sync|vc_wave_table',text)
result={'experimental_engine_and_binding_target_compile':True,'no_old_full_wave_lookup_dependency':True,'no_audio_software_double_helpers':True,'no_atomic_runtime_helpers':True,'experimental_firmware_linked':False,'hardware_tested':False,'pass':True}
pathlib.Path('validation/band-voice-target-prototype6.json').write_text(json.dumps(result,indent=2))
PY
