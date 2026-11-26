"""Check the host MIDI wave-loader recordings (after test34-midi-waves.txt)."""
import json,sys,wave
from pathlib import Path
root=Path(sys.argv[1] if len(sys.argv)>1 else 'build/test34-midi-waves')
checks=[]
for name,silent in [('test34-loading-muted.wav',True),('test34-midi-waves.wav',False)]:
    with wave.open(str(root/name),'rb') as w:
        frames=w.getnframes();pcm=w.readframes(frames)
    zero=not any(pcm)
    assert frames>0 and zero==silent,(name,frames,zero)
    checks.append({'file':name,'frames':frames,'all_zero':zero,'pass':True})
print(json.dumps(checks,indent=2))
