"""Per-wave LZ4 blocks of original float64 coefficients; CRCs of decoded tables."""
from pathlib import Path
import sys,struct,zlib,json,hashlib
p=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(p/'base/tools'))
import lz4blk
raw=(p/'data/wave-coefficients.f64').read_bytes(); tables=(p/'data/source-waves.f32').read_bytes()
header=bytearray(32+64*16);header[:8]=b'VCWAV001';body=bytearray();entries=[]
for w in range(64):
 b=raw[w*2048:(w+1)*2048]; c=lz4blk.compress(b);assert lz4blk.decompress(c)==b
 crc=zlib.crc32(tables[w*114744:(w+1)*114744]); off=len(header)+len(body)
 struct.pack_into('<4I',header,32+w*16,off,len(c),zlib.crc32(b),crc)
 body+=c;entries.append({'wave':w,'bytes':len(c),'table_crc':crc})
struct.pack_into('<4I',header,8,1,64,len(header)+len(body),zlib.crc32(body))
struct.pack_into('<I',header,24,zlib.crc32(header[32:]))
struct.pack_into('<I',header,28,zlib.crc32(header[:28]))
blob=header+body;(p/'data/VIROOOS-WAV.bin').write_bytes(blob)
sines=struct.unpack('<2048d',(p/'data/wave-sines.f64').read_bytes())
text='/* Generated exact source data: tools/generate_wave_bank.cjs + pack_wave_bank.py */\n'
text+='static const double vc_bank_sines[2048]={'+','.join(v.hex() for v in sines)+'};\n'
text+='static const unsigned char vc_bank_seed[]={'+','.join(map(str,blob))+'};\n'
(p/'engine/wave_bank.inc').write_text(text)
report={'format':'VCWAV001: LZ4 original float64 coefficients, per-wave CRCs','waves':64,'bytes':len(blob),'expanded_tables_bytes':len(tables),'two_cache_bytes':2*114744,'sha256':hashlib.sha256(blob).hexdigest(),'roundtrip_coefficients_exact':True,'entries':entries}
(p/'data/wave-bank-manifest.json').write_text(json.dumps(report,indent=2)+'\n')
print('Wave bank bytes:',len(blob),'two caches:',2*114744)
