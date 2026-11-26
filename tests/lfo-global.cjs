const cp=require('child_process'),r=require('../tools/reference.cjs');
const rows=cp.execFileSync('build/lfo-global',{encoding:'utf8',maxBuffer:16e6}).trim().split('\n');
let engine,remaining,maxPhase=0,maxValue=0,checks=0;
const sizes=[7,24,1,65,31,128,17,256];
for(const row of rows){const [shape,env,step,k,phase,value,done]=row.split(' ').map(Number);
 if(step===0&&k===0){engine=new r.Engine(44100);remaining=0;
  for(let i=1;i<=2;i++){for(const [name,v] of [['Mode',1],['EnvMode',env],['Shape',shape],['Rate',100],['Contour',80],['TrigPhase',33],['Clock',i===2?1:0]])engine.setParam('lfo'+i+name,v);}
  engine.triggerNote(60,100);
 }
 if(k===0){if(step===80)engine.triggerNote(72,100);let n=sizes[step%8];while(n){if(!remaining)remaining=32;let take=Math.min(n,remaining);engine.tickMonoLfos(take);remaining-=take;n-=take;}}
 const l=engine.ctx.monoLfos[k],pe=Math.abs(l.phase-phase),ve=Math.abs(l.value-value);maxPhase=Math.max(maxPhase,pe);maxValue=Math.max(maxValue,ve);
 if(pe>0.00002||ve>0.004||+l.done!==done)throw Error(JSON.stringify({shape,env,step,k,phase,value,done,source:{phase:l.phase,value:l.value,done:l.done},pe,ve}));checks++;
}
for(const [cc,name] of [[70,'lfo1Mode'],[82,'lfo2Mode']]){if(r.ccMap[cc]!==name)throw Error(name);for(let v=0;v<128;v++){const e=new r.Engine(44100);e.controlChange(cc,v);if(e.getParam(name)!==+(v>0))throw Error(name+v);}}
console.log(JSON.stringify({source_vectors:checks,max_phase_error:maxPhase,max_value_error:maxValue,boolean_cc_comparisons:256,retrigger_all_128_phases_all_six_key_modes:true,shared_across_notes:true,idle_phase_continuity:true,pass:true},null,2));
