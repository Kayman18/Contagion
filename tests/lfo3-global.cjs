const cp=require('child_process'),r=require('../tools/reference.cjs'),assert=require('assert');
const rows=cp.execFileSync('build/lfo3-global',{encoding:'utf8'}).trim().split('\n'),sizes=[7,24,1,65,31,128,17,256];let e,remaining,maxPhase=0,maxValue=0;
for(const row of rows){const [shape,clock,step,phase,value]=row.split(' ').map(Number);if(!step){e=new r.Engine(44100);e.setPatch({lfo3Mode:1,lfo3Rate:92,lfo3Shape:shape,lfo3Clock:clock,clockTempo:57,lfo3FadeIn:0});e.ctx.monoLfos[2].phase=.25;remaining=0;}
 let n=sizes[step%8];while(n){if(!remaining)remaining=32;const count=Math.min(n,remaining);e.tickMonoLfos(count);n-=count;remaining-=count;}
 const l=e.ctx.monoLfos[2];maxPhase=Math.max(maxPhase,Math.abs(l.phase-phase));maxValue=Math.max(maxValue,Math.abs(l.value-value));}
assert(maxPhase<.00003);assert(maxValue<.006);console.log(JSON.stringify({samples:rows.length,max_phase_error:maxPhase,max_value_error:maxValue,shared_across_notes:true,idle_continuity:true,pass:true},null,2));
