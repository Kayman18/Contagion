const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/osc-volume'),{maxBuffer:6e6});let at=0,max=0,count=0;const results=[];let context, worst;
function check(x){const y=data.readFloatLE(at);at+=4;if(!Number.isFinite(y))throw Error('nonfinite');if(Math.abs(x-y)>max){max=Math.abs(x-y);worst={context,x,y,at};}count++;}
function finish(name,limit){const o={name,values:count,max_error:max,limit,worst,pass:max<limit};results.push(o);if(!o.pass)throw Error(JSON.stringify(o));max=count=0;}
for(let i=0;i<128;i++)check(r.oscVolume(i));finish('source_oscillator_volume_curve',1e-7);
for(let volume=0;volume<128;volume++)for(let route=0;route<4;route++)for(let mode=1;mode<=4;mode++){
 context={volume,route,mode};const e=new r.Engine(44100,{maxVoices:4});e.setPatch({filterRouting:route,filterBalance:64,osc3Mode:mode,osc3Volume:64,osc3Detune:64,osc3Semitone:64,osc1Shape:64,osc2Shape:32,osc2PulseWidth:80,osc2Detune:20,oscBalance:64,oscVolume:volume,saturationCurve:2,subOscVolume:37,ringModVolume:39,cutoff:73,cutoff2:91,filter2CutoffLink:0,filter1Resonance:55,filter2Resonance:70,panorama:64,unisonPanSpread:64,punchIntensity:0});e.noteOn(60,127);
 const v=e.voices.find(v=>v.active);v.control();v.control=()=>{};v.controlCountdown=10000;v.inc[0]=1/128;v.inc[1]=1/64;v.inc[2]=1/256;v.subInc=1/256;
 v.oscillators[0].setShape(64,.5,v.inc[0]);v.oscillators[1].setShape(32,.5*(1-80/127),v.inc[1]);v.oscillators[2].setShape(mode===1?32:mode===2?64:mode===3?127:0,.5*(1-80/127),v.inc[2]);for(const o of v.oscillators)o.reset(0);v.subOscillator.phase=0;
 v.ampEnvelope.next=()=>1;v.filterEnvelope.next=()=>0;let pan=new Float64Array(4),d=route===3?32:0;r.pan(64-d,pan,0);r.pan(64+d,pan,2);for(let i=0;i<4;i++)pan[i]*=.16/.631;v.gainNow.set(pan);v.gainTarget.set(pan);
 const l=new Float32Array(256),rr=new Float32Array(256);v.render(l,rr,0,256);for(let i=0;i<256;i++){check(l[i]);check(rr[i]);}
}
finish('source_full_oscillator_volume_audio_all_routes_and_osc3_modes',5e-5);
if(at!==data.length)throw Error('length mismatch');console.log(JSON.stringify(results,null,2));
