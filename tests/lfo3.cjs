const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/lfo3'),{maxBuffer:8e6});let at=0,max=0,count=0;const results=[];
function read(){const y=data.readFloatLE(at);at+=4;if(!Number.isFinite(y))throw Error('nonfinite');return y;}
function check(x){max=Math.max(max,Math.abs(x-read()));count++;}
function out(v){check(v.inc[0]);check(v.inc[1]);check(v.oscillators[0].duty);check(v.oscillators[1].duty);}
function finish(name,limit){const o={name,values:count,max_error:max,limit,pass:max<limit};results.push(o);if(!o.pass)throw Error(JSON.stringify(o));max=count=0;}
for(let dest=0;dest<6;dest++)for(let amount=0;amount<128;amount++)for(const value of [-1,-.25,0,.5,1]){
 const e=new r.Engine(44100,{maxVoices:1});e.setPatch({lfo3Destination:dest,lfo3OscAmount:amount,lfo3FadeIn:0,lfo1Osc1:80,lfo1Osc2:90,lfo1PulseWidth:80,osc1PulseWidth:32,osc2PulseWidth:96,osc2Detune:20});e.noteOn(60,100);const v=e.voices.find(v=>v.active);v.lfoValue=i=>i===2?value:i===0?.25:0;v.control();out(v);
}
finish('six_lfo3_destinations_all_depth_values',2e-6);
for(let amount=0;amount<128;amount++)for(let t=0;t<6;t++){
 const frame=read(),duration=r.lfo3Fade(amount),gain=duration<=0?1:Math.max(0,Math.min(1,(frame/44100-duration/9)/(duration-duration/9)));check(gain);
}
finish('all_fade_settings_delay_ramp_and_completion',2e-6);
for(let dest=0;dest<6;dest++)for(const clock of [0,5]){
 Math.random=()=>.5;const e=new r.Engine(44100,{maxVoices:1});e.setPatch({lfo3Rate:96,lfo3Shape:0,lfo3Mode:0,lfo3Keyfollow:0,lfo3OscAmount:96,lfo3Destination:dest,lfo3FadeIn:32,lfo3Clock:clock,clockTempo:57,lfo1Rate:64,lfo1Shape:0,lfo1Contour:64,lfo1TrigPhase:1,lfo1Mode:0,lfo1Osc1:80,lfo1PulseWidth:80,osc2Detune:20});e.updateEffects();e.noteOn(60,100);
 const v=e.voices.find(v=>v.active);const l=new Float32Array(1),rr=new Float32Array(1);
 for(let n=0;n<16000;n++){v.render(l,rr,0,1);out(v);check(v.lfos[2].value);check(v.sources[r.sourceIds.Lfo3]);}
}
finish('live_lfo3_fade_clock_and_lfo1_coexistence',.00005);
if(at!==data.length)throw Error('length mismatch');console.log(JSON.stringify(results,null,2));
