const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/lfo2-osc'),{maxBuffer:4e6});let at=0,max=0,scaled=0,count=0;const results=[];
function check(x){const y=data.readFloatLE(at);at+=4;const d=Math.abs(x-y);if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,d);scaled=Math.max(scaled,d/Math.max(1,Math.abs(x)));count++;}
function gains(v){for(let j=0;j<2;j++){check(v.oscillators[j].waveGain);check(v.oscillators[j].sawGain);check(v.oscillators[j].pulseGain);}check(v.fmDepth);check(v.fmScale);}
function finish(name,limit){const out={name,values:count,max_absolute_error:max,max_scaled_error:scaled,limit,pass:scaled<limit};results.push(out);if(!out.pass)throw Error(JSON.stringify(out));max=scaled=count=0;}
for(let a=0;a<128;a++)for(const shape of [0,32,63,64,80,127])for(const value of [-1,-.25,0,.5,1,2]){
 const e=new r.Engine(44100,{maxVoices:1});e.setPatch({osc1Shape:shape,osc2Shape:127-shape,lfo2Shape12:a,lfo2FmAmount:127-a,fmAmount:32,fmFiltEnvAmt:80,fmAmountVelocity:70});e.noteOn(60,100);
 const v=e.voices.find(v=>v.active);v.filterEnvelope.level=.5;v.lfoValue=i=>i===1?value:0;v.control();gains(v);v.lfoValue=()=>0;v.control();gains(v);
}
finish('all_shape_and_fm_amounts_and_neutral_restore',3e-6);
for(const clock of [0,5,17])for(let mode=1;mode<=2;mode++){
 Math.random=()=>.5;const e=new r.Engine(44100,{maxVoices:1});e.setPatch({osc1Shape:32,osc2Shape:96,lfo2Rate:96,lfo2Shape:0,lfo2Contour:64,lfo2TrigPhase:1,lfo2Mode:0,lfo2EnvMode:mode===2?1:0,lfo2Clock:clock,clockTempo:57,lfo2Shape12:112,lfo2FmAmount:80,fmAmount:16,lfo1Rate:64,lfo1Shape:0,lfo1Contour:64,lfo1TrigPhase:1,lfo1Mode:0,lfo1Osc1:80,lfo1PulseWidth:80});e.updateEffects();e.noteOn(60,100);
 const v=e.voices.find(v=>v.active);v.lfoDeviation[0]=v.lfoDeviation[1]=1;const l=new Float32Array(1),rr=new Float32Array(1);
 for(let n=0;n<10000;n++){v.render(l,rr,0,1);gains(v);check(v.lfos[1].rate);check(v.lfos[1].value);check(v.oscillators[0].duty);}
}
finish('live_shape_fm_clock_one_shot_and_lfo1_pwm',.00005);
if(at!==data.length)throw Error('length mismatch');console.log(JSON.stringify(results,null,2));
