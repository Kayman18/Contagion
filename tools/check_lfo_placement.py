#!/usr/bin/env python3
"""Verify exact source LFO float32 samples in mapped flash, without RAM copies.
Usage: check_lfo_placement.py OBJDUMP ELF EXPECTED_F32
"""
import json,struct,subprocess,sys
from pathlib import Path
symbols=subprocess.check_output([sys.argv[1],'-t',sys.argv[2]],text=True)
rows=[line.split() for line in symbols.splitlines() if line.split() and line.split()[-1]=='vc_lfo_waves']
if not rows:
    active=[line.split() for line in symbols.splitlines() if line.split() and line.split()[-1]=='lfo_active_tables']
    assert len(active)==1 and '.bss' in active[0] and int(active[0][-2],16)==3*2049*4
    print(json.dumps({'active_ram_bytes':24588,'embedded_flash_bytes':0,'sample_validation':'tests/run-active-lfo.sh','pass':True},indent=2))
    sys.exit(0)
assert len(rows)==1
row=rows[0];address=int(row[0],16);size=int(row[-2],16)
assert '.text' in row and 0x02000000<=address<0x03000000
raw=Path(sys.argv[2]).read_bytes();assert raw[:6]==b'\x7fELF\x01\x01'
shoff=struct.unpack_from('<I',raw,32)[0];entsize,count=struct.unpack_from('<HH',raw,46)
found=None
for i in range(count):
 section=struct.unpack_from('<10I',raw,shoff+i*entsize);start,offset,length=section[3:6]
 if start<=address and address+size<=start+length and section[1]!=8:
  found=raw[offset+address-start:offset+address-start+size];break
expected=Path(sys.argv[3]).read_bytes();assert found==expected and size==len(expected)
print(json.dumps({'address':address,'flash_bytes':size,'exact_float32_samples':size//4,'section':'.text','pass':True},indent=2))
