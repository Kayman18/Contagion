import sys,json,struct,zlib
from pathlib import Path
sys.path.insert(0,'tools')
from user_patch_sysex import encode,record
out=Path('build/user-sysex');out.mkdir(parents=True,exist_ok=True)
params=bytearray(Path('build/preset-params.bin').read_bytes()[13:139]);ids=json.loads(Path('presets/parameter-map.json').read_text())['native_order'];params[ids.index('VC_CUTOFF')]=53
raw=b'VCP1'+struct.pack('<III',0x49c884bb,126,189)+b'BACKUPTEST'+params+bytes([0,0,64])*11
raw+=struct.pack('<I',zlib.crc32(raw));record(raw);(out/'known.vcp').write_bytes(raw)
for slot in range(3):(out/f'request{slot}.syx').write_bytes(encode(1,slot))
(out/'restore1.syx').write_bytes(encode(3,1,raw));(out/'restore2.syx').write_bytes(encode(3,2,raw))
bad=bytearray(raw);bad[-1]^=1;(out/'invalid.syx').write_bytes(encode(3,1,bad))
