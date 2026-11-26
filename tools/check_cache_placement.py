#!/usr/bin/env python3
"""Verify the target's three-cache split and scratch placement from ELF symbols."""
import json,subprocess,sys
out=subprocess.check_output([sys.argv[1],'-t',sys.argv[2]],text=True)
rows={line.split()[-1]:line.split() for line in out.splitlines() if line.split()}
expected={'cache':('.pool',2*14*2049*4),'cache3_low':('.bss',4*2049*4),'cache3_high':('.pool',10*2049*4),'lfo_active_tables':('.bss',3*2049*4),'accumulator':('.bss',64*8),'load_or_profile':('.bss',max(4096,int(rows['synth'][-2],16)))}
assert "vc_lfo_waves" not in rows
regions={}
for name,(section,size) in expected.items():
 row=rows[name];assert section in row,(name,row)
 assert int(row[-2],16)==size,(name,row)
 regions[name]={'address':int(row[0],16),'bytes':size,'section':section}
ordered=sorted(regions.values(),key=lambda r:r['address'])
for a,b in zip(ordered,ordered[1:]):assert a['address']+a['bytes']<=b['address']
print(json.dumps({'regions':regions,'decoded_wave_bytes':3*14*2049*4,'pass':True},indent=2))
