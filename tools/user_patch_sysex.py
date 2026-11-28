#!/usr/bin/env python3
"""Create request/restore files or extract a native .vcp from a dump. No MIDI I/O."""
import argparse,struct,zlib
from pathlib import Path
HEADER=bytes([0x7d,0x56,0x49,0x52,1])
def record(data):
 if len(data)!=189 or data[:4]!=b'VCP1' or struct.unpack_from('<III',data,4)!=(0x49c884bb,126,189) or zlib.crc32(data[:-4])!=int.from_bytes(data[-4:],'little'):raise ValueError('Invalid or incompatible native patch record')
 return data
def encode(cmd,slot,raw=b''):
 body=bytearray(HEADER+bytes([cmd,slot]))
 for at in range(0,len(raw),7):
  group=raw[at:at+7];body.append(sum((v>>7)<<i for i,v in enumerate(group)));body.extend(v&127 for v in group)
 body.append((-sum(body))&127)
 return b'\xf0'+body+b'\xf7'
def decode(frame):
 if len(frame)<10 or frame[0]!=240 or frame[-1]!=247:raise ValueError('Expected one complete SysEx message')
 p=frame[1:-1]
 if p[:5]!=HEADER or p[6]>31 or any(x>127 for x in p) or sum(p)&127:raise ValueError('Invalid header/checksum')
 raw=bytearray();payload=p[7:-1]
 while payload:
  mask=payload[0];group=payload[1:8]
  if not group or mask>>len(group):raise ValueError('Invalid packing')
  raw.extend(v|((mask>>i&1)<<7) for i,v in enumerate(group));payload=payload[1+len(group):]
 expected={1:0,2:189,3:189,4:1}
 if expected.get(p[5])!=len(raw):raise ValueError('Invalid command length')
 return p[5],p[6],bytes(raw)
def main():
 parser=argparse.ArgumentParser(description=__doc__);sub=parser.add_subparsers(dest='command',required=True)
 p=sub.add_parser('request');p.add_argument('slot',type=int,choices=range(1,33));p.add_argument('output')
 p=sub.add_parser('restore');p.add_argument('slot',type=int,choices=range(1,33));p.add_argument('input');p.add_argument('output')
 p=sub.add_parser('extract');p.add_argument('input');p.add_argument('output')
 p=sub.add_parser('inspect');p.add_argument('input')
 a=parser.parse_args()
 if a.command=='request':Path(a.output).write_bytes(encode(1,a.slot-1));return
 if a.command=='restore':
  data=Path(a.input).read_bytes()
  if data[:1]==b'\xf0':cmd,slot,data=decode(data);assert cmd==2,'Expected a dump response'
  Path(a.output).write_bytes(encode(3,a.slot-1,record(data)));return
 cmd,slot,data=decode(Path(a.input).read_bytes())
 if a.command=='extract':
  if cmd!=2:raise ValueError('Expected a dump response')
  Path(a.output).write_bytes(record(data));return
 if cmd==4:print('Slot',slot+1,'status',data[0],{0:'saved',1:'busy/retry',2:'empty',3:'invalid',4:'write failed'}.get(data[0],'unknown'))
 else:print('Slot',slot+1,'command',cmd,'name',record(data)[16:26].decode('ascii').rstrip() if data else '')
if __name__=='__main__':main()
