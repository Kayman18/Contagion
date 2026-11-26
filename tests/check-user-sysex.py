import sys,json
from pathlib import Path
sys.path.insert(0,'tools');from user_patch_sysex import decode,record
p=Path('build/user-sysex')
for name,status in [('busy',1),('saved',0),('invalid-reply',3),('failed',4),('empty',2)]:
 cmd,slot,raw=decode((p/(name+'.syx')).read_bytes());assert cmd==4 and raw==bytes([status]),(name,cmd,slot,raw)
cmd,slot,raw=decode((p/'dump1.syx').read_bytes());assert (cmd,slot)==(2,1) and record(raw)==(p/'known.vcp').read_bytes()
cmd,slot,raw=decode((p/'dump0.syx').read_bytes());assert (cmd,slot)==(2,0);record(raw)
Path('validation/band18-transfer.json').write_text(json.dumps({'export_valid':True,'import_exact':True,'invalid_record_rejected':True,'failed_write_acknowledged':True,'empty_slot_acknowledged':True,'pass':True},indent=2))
