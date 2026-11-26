const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/osc3-waves'),{maxBuffer:2e6});let at=0,max=0,count=0;const results=[];
function check(x){const y=data.readFloatLE(at);at+=4;if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,Math.abs(x-y));count++;}
function finish(name,limit){const o={name,values:count,max_error:max,limit,pass:max<limit};results.push(o);if(!o.pass)throw Error(JSON.stringify(o));max=count=0;}
for(let mode=1;mode<=7;mode++)for(const note of [24,60,96])for(const tune of [16,64,112])for(const detune of [0,64,127])for(const shape of [0,64,127])for(const value of [-1,0,1]){
 const e=new r.Engine(44100,{maxVoices:4});e.setPatch({osc3Mode:mode,osc3Volume:64,osc3Semitone:tune,osc3Detune:detune,osc2Shape:shape,osc2Wave:3,osc2PulseWidth:80,osc2Semitone:70,osc2Detune:20,osc2FiltEnvAmt:80,lfo1Osc2:80,lfo1PulseWidth:80,lfo2Shape12:96,oscBalance:64,oscVolume:64});e.noteOn(note,100);
 const v=e.voices.find(v=>v.active);v.filterEnvelope.level=.5;v.lfoValue=i=>i<2?value:0;v.control();check(v.inc[2]);check(v.oscillators[2].waveGain);check(v.oscillators[2].sawGain);check(v.oscillators[2].pulseGain);check(v.oscillators[2].duty);check(v.gain.osc3);
}
finish('source_osc3_modes_tuning_modulation_gain_and_pwm',3e-6);
for(let route=0;route<4;route++)for(let mode=1;mode<=7;mode++){
 const e=new r.Engine(44100,{maxVoices:4});e.setPatch({filterRouting:route,filterBalance:64,osc3Mode:mode,osc3Volume:64,osc3Detune:64,osc3Semitone:64,osc1Shape:64,osc2Shape:32,osc2PulseWidth:80,osc2Detune:20,oscBalance:64,oscVolume:64,cutoff:73,cutoff2:91,filter2CutoffLink:0,filter1Resonance:55,filter2Resonance:70,panorama:64,unisonPanSpread:64,punchIntensity:0});e.noteOn(60,127);
 const v=e.voices.find(v=>v.active);v.control();v.control=()=>{};v.controlCountdown=10000;v.inc[0]=1/128;v.inc[1]=1/64;v.inc[2]=1/256;
 v.oscillators[0].setShape(64,.5,v.inc[0]);v.oscillators[1].setShape(32,.5*(1-80/127),v.inc[1]);v.oscillators[2].setShape(mode===1?32:mode===2?64:mode===3?127:0,.5*(1-80/127),v.inc[2]);for(const o of v.oscillators)o.reset(0);
 v.ampEnvelope.next=()=>1;v.filterEnvelope.next=()=>0;let pan=new Float64Array(4),d=route===3?32:0;r.pan(64-d,pan,0);r.pan(64+d,pan,2);for(let i=0;i<4;i++)pan[i]*=.16/.631;v.gainNow.set(pan);v.gainTarget.set(pan);
 const l=new Float32Array(2048),rr=new Float32Array(2048);v.render(l,rr,0,2048);for(let i=0;i<2048;i++){check(l[i]);check(rr[i]);}
}
finish('source_osc3_audio_all_filter_routes',5e-5);
if(at!==data.length)throw Error('length mismatch');console.log(JSON.stringify(results,null,2));
