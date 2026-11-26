const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/saturation-fractional'),{maxBuffer:4e6});let at=0,max=0,count=0;const results=[];let periodRelativeMax=0;
function check(x){const y=data.readFloatLE(at);at+=4;if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,Math.abs(x-y));count++;return y;}
function finish(name,limit){const o={name,values:count,max_error:max,limit,pass:max<limit};results.push(o);if(!o.pass)throw Error(JSON.stringify(o));max=count=0;}
for(let curve=0;curve<=14;curve++)for(let level=64.375;level<127;level++)for(const note of [-36,0,36]){
 const s=new r.Saturation(44100);s.set(curve,level,note);
 if(curve===9||curve===10){const period=data.readFloatLE(at);at+=4;periodRelativeMax=Math.max(periodRelativeMax,Math.abs(period-s.period)/s.period);if(!Number.isFinite(period)||periodRelativeMax>1e-6)throw Error('period precision');s.period=Math.round(period*16)/16;}
 else if(curve>=11)check(s.coef);
 for(let i=0;i<256;i++)check(s.process((i%67-33)/16));
}
finish('all_saturation_modes_levels_and_follow_coefficients',.00002);results.push({name:'rate_period_relative_error',max_error:periodRelativeMax,limit:1e-6,pass:true});
for(let route=0;route<4;route++)for(let curve=0;curve<=14;curve++){
 const e=new r.Engine(44100,{maxVoices:4});e.setPatch({filterRouting:route,filterBalance:64,saturationCurve:curve,oscVolume:96,cutoff:73,cutoff2:91,filter2CutoffLink:0,filter1Resonance:55,filter2Resonance:70,panorama:64,unisonPanSpread:64,punchIntensity:0});e.noteOn(60,127);const v=e.voices.find(v=>v.active);v.control();v.control=()=>{};v.controlCountdown=10000;
 if(curve===9||curve===10)v.saturation.period=Math.round(Math.fround(v.saturation.period)*16)/16;
 let a=0,b=0;v.oscillators[0].next=()=>((a++%31)-15)/16;v.oscillators[1].next=()=>((b++%23)-11)/16;Object.assign(v.gain,{osc1:1,osc2:1,osc3:0,noise:0,sub:0,ring:0,filter:1});v.punchAt=1e9;v.ampEnvelope.next=()=>1;v.filterEnvelope.next=()=>0;
 const pan=new Float64Array(4),d=route===3?32:0;r.pan(64-d,pan,0);r.pan(64+d,pan,2);v.gainNow.set(pan);v.gainTarget.set(pan);
 const l=new Float32Array(1024),rr=new Float32Array(1024);v.render(l,rr,0,1024);for(let i=0;i<1024;i++){check(l[i]);check(rr[i]);}
}
finish('source_filter1_saturation_audio_all_routes',.0001);
if(at!==data.length)throw Error('length mismatch');console.log(JSON.stringify(results,null,2));
