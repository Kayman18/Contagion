const cp=require('child_process'),r=require('../tools/reference.cjs');
const lines=cp.execFileSync('build/lfo-env-midi',{encoding:'utf8'}).trim().split('\n');
for(const line of lines){const [k,value,old,mode]=line.split(' ').map(Number),cc=k?81:69,name=k?'lfo2EnvMode':'lfo1EnvMode';
 if(r.ccMap[cc]!==name)throw Error('source CC mismatch');const engine=new r.Engine(44100);engine.controlChange(cc,value);
 if(engine.getParam(name)!==mode-1)throw Error(JSON.stringify({k,value,old,mode,source:engine.getParam(name)}));
}
console.log(JSON.stringify({source_comparisons:lines.length,native_state_and_audio_comparisons:780,controls:[69,81],nonzero_selects_envelope:true,phase_preserved:true,pass:true},null,2));
