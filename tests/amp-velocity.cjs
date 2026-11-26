const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/amp-velocity'),{maxBuffer:2e6});let at=0,max=0,count=0;const results=[];
function check(x){const y=data.readFloatLE(at);at+=4;if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,Math.abs(x-y));count++;}
function finish(name,limit){const o={name,values:count,max_error:max,limit,pass:max<limit};results.push(o);if(!o.pass)throw Error(JSON.stringify(o));max=count=0;}
for(let amount=0;amount<=128;amount++)for(let velocity=0;velocity<=127;velocity++)check(amount===128?velocity/127:r.ampVelocity(amount,velocity));finish('original_amp_velocity_curve_all_settings_and_velocities',2e-6);
for(let route=0;route<4;route++)for(const amount of [0,32,64,96,127,128])for(const velocity of [1,32,64,100,127]){
 const e=new r.Engine(44100,{maxVoices:4});e.setPatch({filterRouting:route,filterBalance:64,osc3Mode:2,ampVelocity:Math.min(amount,127),osc3Volume:64,osc3Detune:64,osc3Semitone:64,osc1Shape:64,osc2Shape:32,osc2PulseWidth:80,osc2Detune:20,oscBalance:64,oscVolume:64,cutoff:73,cutoff2:91,filter2CutoffLink:0,filter1Resonance:55,filter2Resonance:70,panorama:64,unisonPanSpread:64,punchIntensity:0});e.noteOn(60,velocity);
 const v=e.voices.find(v=>v.active);v.control();v.control=()=>{};v.controlCountdown=10000;v.inc[0]=1/128;v.inc[1]=1/64;v.inc[2]=1/256;
 v.oscillators[0].setShape(64,.5,v.inc[0]);v.oscillators[1].setShape(32,.5*(1-80/127),v.inc[1]);v.oscillators[2].setShape(64,.5*(1-80/127),v.inc[2]);for(const o of v.oscillators)o.reset(0);
 v.punch=0;v.ampEnvelope.next=()=>1;v.filterEnvelope.next=()=>0;let pan=new Float64Array(4),d=route===3?32:0;r.pan(64-d,pan,0);r.pan(64+d,pan,2);for(let i=0;i<4;i++)pan[i]*=.16/.631*(amount===128?velocity/127:r.ampVelocity(amount,velocity));v.gainNow.set(pan);v.gainTarget.set(pan);
 const l=new Float32Array(256),rr=new Float32Array(256);v.render(l,rr,0,256);for(let i=0;i<256;i++){check(l[i]);check(rr[i]);}
}
finish('amp_velocity_stereo_audio_all_routes',5e-5);
for(let amount=0;amount<=128;amount++)for(const velocity of [1,32,64,100,127]){
 const start=velocity/127,target=amount===128?start:r.ampVelocity(amount,velocity);
 for(let i=0;i<40;i++)check(start+(target-start)*Math.min(1,(i+1)/32));
}
finish('native_32_sample_gain_transition',2e-6);
if(at!==data.length)throw Error('length mismatch');console.log(JSON.stringify(results,null,2));
