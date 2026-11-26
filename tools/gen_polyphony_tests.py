#!/usr/bin/env python3
"""Write tests/polyphony-8.txt and tests/polyphony-12.txt (simulator scripts).
Full chords, a steal, 2-layer unison (AutoBendBC, C0) and 4-layer unison (J.Edgar HS, D53)."""
from pathlib import Path
def sel(bank,prog): return [f"midi USB B0 00 {bank:02X}",f"midi USB C0 {prog:02X}","wait 12000","expect wave_state 0"]
notes=[36,40,43,47,50,53,57,60,64,67,71,74]
for voices in (8,12):
    L=["wait 12000","expect wave_state 0",f"wav poly{voices}-init.wav"]
    for n in notes[:voices]: L+=[f"midi USB 90 {n:02X} 64","wait 20"]
    L+=["wait 400",f"expect voices {voices}","midi USB 90 4D 64","wait 200",f"expect voices {voices}"]
    L+=[f"midi USB 80 {n:02X} 00" for n in notes[:voices]+[77]]+["wait 3000","expect voices 0","wavstop"]
    L+=sel(0,0)+[f"wav poly{voices}-unison2.wav"]
    for n in notes[:voices//2]: L+=[f"midi USB 90 {n+12:02X} 64","wait 20"]
    L+=["wait 500",f"expect voices {voices}","midi USB 90 5A 64","wait 300",f"expect voices {voices}"]
    L+=[f"midi USB 80 {n+12:02X} 00" for n in notes[:voices//2]]+["midi USB 80 5A 00","wait 4000","wavstop"]
    L+=sel(1,53)+[f"wav poly{voices}-unison4.wav"]
    chord=[48,55,64][:voices//4]
    for n in chord: L+=[f"midi USB 90 {n:02X} 64","wait 20"]
    L+=["wait 500",f"expect voices {voices}","midi USB 90 43 64","wait 300",f"expect voices {voices}"]
    L+=[f"midi USB 80 {n:02X} 00" for n in chord+[0x43]]+["wait 4000","wavstop","expect queue_lost 0"]
    Path(f"tests/polyphony-{voices}.txt").write_text("\n".join(L)+"\n")
# Overload shedding with 8 notes (valid for both voice counts).
L=["wait 12000","expect wave_state 0"]+[f"midi USB 90 {n:02X} 64" for n in notes[:8]]+["wait 300","expect voices 8"]
L+=[f"midi USB 80 {n:02X} 00" for n in notes[:8]]+["wait 2","expect shed_now 0","wait 30","expect shed_count 1","expect voices 7","expect shed_now 0","wait 30","expect shed_count 2","expect voices 6","wait 4000","expect voices 0"]
Path("tests/overload-shed.txt").write_text("\n".join(L)+"\n")
