#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
: "${JIELI_TOOLCHAIN:?Set JIELI_TOOLCHAIN}"
flags=(-target pi32v2 -isystem "$JIELI_TOOLCHAIN/pi32v2/include" -mcpu=r3 -mfprev1 -ffp-contract=off -std=c11 -O2 -fno-common -DVC_BAND_FM1_SINGLE_CORE)
for unit in band_bank band_rebuild band_cache band_handoff; do
 "$JIELI_TOOLCHAIN/pi32v2/bin/clang" "${flags[@]}" -c "experimental/$unit.c" -o "build/$unit-fm1.o"
 "$JIELI_TOOLCHAIN/common/bin/objdump" -t "build/$unit-fm1.o" > "validation/$unit-fm1-symbols.txt"
done
python3 - <<'PY'
import pathlib,json
units=['band_bank','band_rebuild','band_cache','band_handoff'];r={}
for u in units:
 text=pathlib.Path('validation/'+u+'-fm1-symbols.txt').read_text()
 assert '__atomic' not in text and '__sync' not in text
 assert 'malloc' not in text and 'calloc' not in text
 r[u]=[line.split()[-1] for line in text.splitlines() if '*UND*' in line and len(line.split())>3]
result={'target_objects_compiled':units,'no_atomic_runtime_helpers':True,'no_heap_allocation':True,'undefined_symbols':r,'firmware_linked':False,'hardware_tested':False,'pass':True}
pathlib.Path('validation/band-background-target-prototype5.json').write_text(json.dumps(result,indent=2));print(json.dumps(result))
PY
