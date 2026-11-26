const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/lfo2-pan'),{maxBuffer:8e6});let at=0,max=0,count=0;const results=[];
function check(x){const y=data.readFloatLE(at);at+=4;const d=Math.abs(x-y);if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,d);count++;}
function finish(name,limit){const o={name,values:count,max_error:max,limit,pass:max<limit};results.push(o);if(!o.pass)throw Error(JSON.stringify(o));max=count=0;}
function normalizePan(v,route,pan,spread,value,amount){
 const p=pan+2*(amount-64)*value,expected=new Float64Array(4),d=route===3?spread/2:0;r.pan(p-d,expected,0);r.pan(p+d,expected,2);
 const factor=(v.gainTarget[0]+v.gainTarget[1])/(expected[0]+expected[1]);for(let i=0;i<4;i++)v.gainTarget[i]/=factor;
}
for(let route=0;route<4;route++)for(let mode=0;mode<4;mode++)for(const amount of [0,32,64,96,127])for(const pan of [0,64,127])for(const spread of [0,64,127]){
 const e=new r.Engine(44100,{maxVoices:1});e.setPatch({filterRouting:route,filterBalance:64,filter1Mode:mode,filter2Mode:(mode+1)%4,cutoff:73,cutoff2:91,filter2CutoffLink:0,filter1Resonance:55,filter2Resonance:70,panorama:pan,unisonPanSpread:spread,lfo2Panorama:amount,punchIntensity:0});e.noteOn(60,127);
 const v=e.voices.find(v=>v.active),orig=v.control.bind(v);let step=0;v.control=()=>{const value=[-1,-.25,.5,1][step];v.lfoValue=i=>i===1?value:0;orig();normalizePan(v,route,pan,spread,value,amount);if(!step)v.gainNow.set(v.gainTarget);step++;Object.assign(v.gain,{osc1:1,osc2:1,osc3:0,noise:0,sub:0,ring:0,filter:1});};
 let a=0,b=0;v.oscillators[0].next=()=>((a++%31)-15)/128;v.oscillators[1].next=()=>((b++%23)-11)/128;v.punchAt=1e9;v.ampEnvelope.next=()=>1;v.filterEnvelope.next=()=>0;
 v.controlCountdown=0;const l=new Float32Array(1),rr=new Float32Array(1);for(let n=0;n<128;n++){l.fill(0);rr.fill(0);v.render(l,rr,0,1);check(l[0]);check(rr[0]);for(let i=0;i<4;i++)check(v.gainNow[i]);}
}
finish('source_pan_slew_and_stereo_audio_all_routes_modes_and_edges',2e-5);
for(const route of [0,3])for(let shape=0;shape<4;shape++){
 Math.random=()=>.5;const e=new r.Engine(44100,{maxVoices:1});e.setPatch({filterRouting:route,panorama:64,unisonPanSpread:64,lfo2Panorama:96,lfo2Rate:96,lfo2Shape:shape,lfo2Contour:64,lfo2TrigPhase:1,lfo2Mode:0});e.noteOn(60,100);
 const v=e.voices.find(v=>v.active),orig=v.control.bind(v);v.lfoDeviation[1]=1;normalizePan(v,route,64,64,0,96);v.gainNow.set(v.gainTarget);
 v.control=()=>{const value=v.lfoValue(1);orig();normalizePan(v,route,64,64,value,96);};
 const l=new Float32Array(1),rr=new Float32Array(1);for(let n=0;n<12000;n++){v.render(l,rr,0,1);for(let i=0;i<4;i++)check(v.gainNow[i]);}
}
finish('live_lfo2_pan_control_timing',5e-5);
if(at!==data.length)throw Error('length mismatch');console.log(JSON.stringify(results,null,2));
