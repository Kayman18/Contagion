#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
: "${JIELI_TOOLCHAIN:?Set JIELI_TOOLCHAIN}"
flags=(-target pi32v2 -isystem "$JIELI_TOOLCHAIN/pi32v2/include" -mcpu=r3 -mfprev1 -ffp-contract=off -std=c11 -O2 -fno-common -DVC_BAND_FM1_SINGLE_CORE)
"$JIELI_TOOLCHAIN/pi32v2/bin/clang" "${flags[@]}" -c experimental/band_handoff.c -o build/band-handoff-fm1.o
"$JIELI_TOOLCHAIN/pi32v2/bin/clang" "${flags[@]}" -c tests/band-target-layout.c -o build/band-target-layout.o
"$JIELI_TOOLCHAIN/common/bin/objdump" -d -r build/band-handoff-fm1.o > validation/band-handoff-fm1-disassembly.txt
"$JIELI_TOOLCHAIN/common/bin/objdump" -t build/band-handoff-fm1.o > validation/band-handoff-fm1-symbols.txt
"$JIELI_TOOLCHAIN/common/bin/objdump" -t build/band-target-layout.o > validation/band-target-layout.txt
python3 - <<'PY'
import json,re,pathlib
root=pathlib.Path('validation');symbols=(root/'band-handoff-fm1-symbols.txt').read_text();dis=(root/'band-handoff-fm1-disassembly.txt').read_text();layout=(root/'band-target-layout.txt').read_text()
assert '__atomic' not in symbols and '__sync' not in symbols
checked=[]
for name in ['vc_band_audio_request','vc_band_audio_release','vc_band_audio_boundary']:
 body=dis.split(name+':\n',1)[1].split('\n\n',1)[0]
 assert not re.search(r'\bcall\b',body),name
 assert 'csync' in body,name
 checked.append(name)
sizes={}
for line in layout.splitlines():
 if 'vc_band_target_' in line:
  parts=line.split();sizes[parts[-1]]=int(parts[-2],16)
assert sizes['vc_band_target_mailbox_alignment']>=4
result={'target':'pi32v2/r3','atomic_helpers':False,'call_free_audio_functions':checked,'csync_present':True,'sizes_bytes':sizes,'firmware_linked':False,'hardware_tested':False,'pass':True}
(root/'band-target-publication-prototype4.json').write_text(json.dumps(result,indent=2));print(json.dumps(result))
PY
