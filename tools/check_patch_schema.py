from pathlib import Path
import re
root=Path(__file__).resolve().parent.parent
s=(root/'engine/virooos.h').read_text()
ids=re.findall(r'VC_[A-Z0-9_]+',s[s.index('VC_SHAPE1'):s.index('VC_PARAMS')])
h=2166136261
for b in ','.join(ids).encode():h=((h^b)*16777619)&0xffffffff
expected=int(re.search(r'0x([0-9a-f]+)u',(root/'experimental/patch_schema.inc').read_text())[1],16)
assert h==expected,'Native parameter order changed: explicitly version/migrate patch storage before shipping'
print(f'patch schema verified: {h:08x}, {len(ids)} native controls')
