const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/punch'),{maxBuffer:2e6});let at=0,max=0,count=0;const results=[];
function check(x){const y=data.readFloatLE(at);at+=4;if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,Math.abs(x-y));count++;}
function finish(name,limit){const o={name,values:count,max_error:max,limit,pass:max<limit};results.push(o);if(!o.pass)throw Error(JSON.stringify(o));max=count=0;}
for(let i=0;i<128;i++)check(r.punchLevel(i));finish('source_punch_intensity_curve',2e-7);
for(const level of [0,1,32,64,96,127])for(const volume of [0,32,64,96,127])for(let route=0;route<4;route++)for(const curve of [0,2,9,11,13]){
 const e=new r.Engine(44100,{maxVoices:4});e.setPatch({filterRouting:route,filterBalance:64,saturationCurve:curve,oscVolume:volume,punchIntensity:level,cutoff:73,cutoff2:91,filter2CutoffLink:0,filter1Resonance:55,filter2Resonance:70,panorama:64,unisonPanSpread:64});e.noteOn(60,127);const v=e.voices.find(v=>v.active);v.control();v.control=()=>{};v.controlCountdown=10000;if(level===0)v.punch=0; /* Native explicit OFF; source zero has a tiny residual. */
 if(curve===9)v.saturation.period=Math.round(Math.fround(v.saturation.period)*16)/16;
 v.oscillators[0].next=()=>0;v.oscillators[1].next=()=>0;Object.assign(v.gain,{osc1:0,osc2:0,osc3:0,noise:0,sub:0,ring:0,filter:1});v.ampEnvelope.next=()=>1;v.filterEnvelope.next=()=>0;
 const pan=new Float64Array(4),d=route===3?32:0;r.pan(64-d,pan,0);r.pan(64+d,pan,2);v.gainNow.set(pan);v.gainTarget.set(pan);
 const l=new Float32Array(256),rr=new Float32Array(256);v.render(l,rr,0,256);for(let i=0;i<256;i++){check(l[i]);check(rr[i]);}
}
finish('source_punch_audio_all_routes_volume_ranges_and_saturation_families',5e-5);
if(at!==data.length)throw Error('length mismatch');console.log(JSON.stringify(results,null,2));
