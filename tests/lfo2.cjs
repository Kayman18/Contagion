const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/lfo2'),{maxBuffer:4e6});let at=0,max=0,scaled=0,count=0;const results=[];
function check(x){const y=data.readFloatLE(at);at+=4;const d=Math.abs(x-y);if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,d);scaled=Math.max(scaled,d/Math.max(1,Math.abs(x)));count++;}
function finish(name,limit){const out={name,values:count,max_absolute_error:max,max_scaled_error:scaled,limit,pass:scaled<limit};results.push(out);if(!out.pass)throw Error(JSON.stringify(out));max=scaled=count=0;}
for(let link=0;link<2;link++)for(const note of [24,60,96])for(let amount=0;amount<128;amount++)for(const value of [-1,-.25,0,.5,1,2]){
 const e=new r.Engine(44100,{maxVoices:1});e.setPatch({cutoff:73,cutoff2:91,filter2CutoffLink:link,filter1EnvAmt:70,filter2EnvAmt:81,filter1EnvPolarity:1,filter2EnvPolarity:1,filter1Keyfollow:90,filter2Keyfollow:33,filterKeytrackBase:60,lfo2Cutoff1:amount,lfo2Cutoff2:127-amount});e.noteOn(note,100);
 const v=e.voices.find(v=>v.active);v.filterEnvelope.level=Math.fround(.3);v.lfoValue=i=>i===1?value:0;v.control();check(v.filter1.cutoff);check(v.filter2.cutoff);
}
finish('lfo2_cutoff_curves_link_keytrack_and_envelope',2e-5);
for(let shape=0;shape<4;shape++)for(let mode=1;mode<=2;mode++){
 Math.random=()=>.5;const e=new r.Engine(44100,{maxVoices:1});e.setPatch({cutoff:55,cutoff2:75,filter2CutoffLink:0,lfo2Rate:96,lfo2Shape:shape,lfo2Contour:64,lfo2TrigPhase:1,lfo2EnvMode:mode===2?1:0,lfo2Mode:0,lfo2Cutoff1:96,lfo2Cutoff2:32,lfo1Rate:64,lfo1Shape:0,lfo1Contour:64,lfo1TrigPhase:1,lfo1Mode:0,lfo1Osc1:80,lfo1Resonance:80});e.noteOn(60,100);
 const v=e.voices.find(v=>v.active);v.lfoDeviation[0]=v.lfoDeviation[1]=1;const l=new Float32Array(1),rr=new Float32Array(1);
 for(let i=0;i<12000;i++){v.render(l,rr,0,1);check(v.lfos[1].value);check(v.filter1.cutoff);check(v.filter2.cutoff);check(v.filter1.g/(1+v.filter1.g));check(v.filter2.g/(1+v.filter2.g));check(v.lfos[0].value);check(v.inc[0]);}
}
finish('live_dual_lfo_control_timing_and_cutoff_coefficients',.00005);
if(at!==data.length)throw Error('length mismatch');console.log(JSON.stringify(results,null,2));
