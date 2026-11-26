const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/filter-velocity'),{maxBuffer:5e6});let at=0,max=0,count=0;const results=[];
function check(x){const y=data.readFloatLE(at);at+=4;if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,Math.abs(x-y));count++;}
function finish(name,limit){const o={name,values:count,max_error:max,limit,pass:max<limit};results.push(o);if(!o.pass)throw Error(JSON.stringify(o));max=count=0;}
for(let i=0;i<=2048;i++)check(r.envAmount(i/16));finish('fractional_envelope_amount_including_127_boundary',1e-6);
for(let route=0;route<4;route++)for(let amount=0;amount<128;amount++)for(const velocity of [1,32,64,100,127])for(const env of [0,.5,1]){
 const e=new r.Engine(44100,{maxVoices:4});e.setPatch({filterRouting:route,filterBalance:64,filter1Mode:route,filter2Mode:(route+1)%4,cutoff:32,cutoff2:96,filter2CutoffLink:0,filter1Resonance:55,filter2Resonance:70,filter1EnvAmt:96,filter2EnvAmt:24,filter1EnvPolarity:1,filter2EnvPolarity:0,filter1EnvAmtVelocity:amount,filter2EnvAmtVelocity:127-amount,resonance1Velocity:amount,resonance2Velocity:127-amount,lfo1Resonance:80,panorama:64,unisonPanSpread:64,punchIntensity:0});e.noteOn(60,velocity);
 const v=e.voices.find(v=>v.active);v.filterEnvelope.level=env;v.lfoValue=i=>i===0?.25:0;v.control();check(v.filter1.cutoff);check(v.filter2.cutoff);check(v.filter1.resonance);check(v.filter2.resonance);
 v.control=()=>{};v.controlCountdown=10000;let a=0,b=0;v.oscillators[0].next=()=>((a++%31)-15)/128;v.oscillators[1].next=()=>((b++%23)-11)/128;Object.assign(v.gain,{osc1:1,osc2:1,osc3:0,noise:0,sub:0,ring:0,filter:1});v.fmDepth=0;v.punchAt=1e9;v.ampEnvelope.next=()=>1;v.filterEnvelope.next=()=>0;
 const pan=new Float64Array(4),d=route===3?32:0;r.pan(64-d,pan,0);r.pan(64+d,pan,2);v.gainNow.set(pan);v.gainTarget.set(pan);
 const l=new Float32Array(64),rr=new Float32Array(64);v.render(l,rr,0,64);for(let i=0;i<64;i++){check(l[i]);check(rr[i]);}
}
finish('source_filter_velocity_controls_and_audio_all_routes',8e-5);
if(at!==data.length)throw Error('length mismatch');console.log(JSON.stringify(results,null,2));
