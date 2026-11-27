#!/usr/bin/env python3
"""Decode the produced FWSC and check its CRCs and exact app/loader payloads."""
import argparse, hashlib, json, struct, sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'base/tools'))
import fm1pkg_make as pkg

def require(ok,msg):
    if not ok: raise ValueError(msg)

def crc_entry(data):
    require(len(data)==32,'truncated directory entry')
    require(pkg.crc16(data[2:])==struct.unpack_from('<H',data)[0],'directory entry CRC')
    return struct.unpack_from('<HII',data,2)

def validate(raw,app,ota,cache=None):
    require(len(raw)>=960,'truncated FWSC')
    logical=bytearray();identity=[];ended=False
    for i in range(20):
        logical+=raw[i*48:i*48+47];m=raw[i*48+47]
        if m==0x7d: ended=True
        else:
            require(not ended,'identity after terminator');identity.append(chr((m-i-1)&255))
    logical+=raw[960:]
    header=bytearray(logical[:64]);pkg.enc(header,0,64)
    require(pkg.crc16(header[2:])==struct.unpack_from('<H',header)[0],'UFW header CRC')
    total,count=struct.unpack_from('<IH',header,4)
    require(total==len(logical),'UFW size')
    require(count==2,'UFW entry count')
    require(header[16:22]==b'AC791N','chip family')
    entries=logical[64:64+count*80]
    require(pkg.crc16(entries)==struct.unpack_from('<H',header,2)[0],'UFW directory CRC')
    parts={};spans=[]
    for i in range(count):
        e=bytearray(entries[i*80:(i+1)*80]);pkg.enc(e,0,80)
        typ,index,crc,_,off,size,padded=struct.unpack_from('<HHHHIII',e)
        name=bytes(e[64:]).split(b'\0')[0].decode('ascii')
        require(off>=1024 and off%256==0 and off+size<=len(logical),'payload bounds/alignment')
        require(index==i and padded==(size+31)&~31,'payload metadata')
        require(all(off>=b or off+size<=a for a,b in spans),'overlapping payloads')
        spans.append((off,off+size));p=bytes(logical[off:off+size])
        require(pkg.crc16(p)==crc,'payload CRC: '+name);parts[name]=(typ,p)
    require(set(parts)=={'flash.bin','ota.bin'},'unexpected payloads')
    require(parts['ota.bin']==(100,ota),'loader differs from linked loader')
    typ,flash=parts['flash.bin'];require(typ==0 and len(flash)==(0xDC000 if cache is not None else pkg.FLASH_SIZE),'flash layout/size')
    if cache is not None:
        require(flash[pkg.FLASH_SIZE:0x97000] == b'\xff' * (0x97000-pkg.FLASH_SIZE),'reserved gap')
        require(flash[0x97000:] == cache,'wave cache payload')
    config=bytearray(flash[64:96]);pkg.enc(config,0,32);crc,pos,size=crc_entry(config)
    require(bytes(config[16:30])==b'isd_config.ini','configuration name')
    cfg=flash[pos:pos+size];require(pkg.crc16(cfg)==crc,'config payload CRC')
    key=pkg.chipkey_decode(cfg[:32])
    region=bytearray(flash[0x4000:]);pkg.sfc(region,0,len(region),0,key)
    crc,addr,size=crc_entry(region[:32]);require(addr==0x02000120 and 32<size<=len(region),'app directory bounds')
    require(pkg.crc16(region[32:size])==crc,'app directory body CRC')
    crc,pos,size=crc_entry(region[32:64]);require(region[48:55]==b'app.bin','app name')
    require(pos==0x120 and size==pkg.APP_SLOT,'app entry bounds')
    payload=bytes(region[pos:pos+size]);require(pkg.crc16(payload)==crc,'app payload CRC')
    require(payload==app+b'\xff'*(pkg.APP_SLOT-len(app)),'app differs from linked binary')
    require(app[:4]==bytes.fromhex('04818000'),'entry instruction')
    require(len(app)<=pkg.APP_SLOT,'app exceeds slot')
    return {'identity':''.join(identity),'fwsc_bytes':len(raw),'application_bytes':len(app),'app_slot_bytes':pkg.APP_SLOT,
      'loader_bytes':len(ota),'all_crcs_valid':True,'decrypted_app_exact':True,'loader_exact':True,
      'entry_stub_valid':True,'fwsc_sha256':hashlib.sha256(raw).hexdigest(),'app_sha256':hashlib.sha256(app).hexdigest(),
      'hardware_tested':False}

def main():
    p=argparse.ArgumentParser();p.add_argument('fwsc',type=Path);p.add_argument('app',type=Path);p.add_argument('loader',type=Path);p.add_argument('--wave-cache',type=Path)
    a=p.parse_args();raw=a.fwsc.read_bytes();app=a.app.read_bytes();ota=a.loader.read_bytes();cache=a.wave_cache.read_bytes() if a.wave_cache else None;r=validate(raw,app,ota,cache)
    if cache is not None:r['wave_cache_exact']=True
    for at in (0,len(raw)//2,len(raw)-1):
        damaged=bytearray(raw);damaged[at]^=1
        try: validate(damaged,app,ota,cache)
        except ValueError: pass
        else: raise ValueError('validator accepted damaged package')
    r['corruption_detection_checks']=3
    print(json.dumps(r,indent=2))
if __name__=='__main__':main()
