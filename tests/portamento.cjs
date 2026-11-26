const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/portamento'),{maxBuffer:6e6});let at=0,max=0,count=0;const results=[];
function check(x){const y=data.readFloatLE(at);at+=4;if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,Math.abs(x-y));count++;}
function finish(name,limit){const o={name,values:count,max_error:max,limit,pass:max<limit};results.push(o);if(!o.pass)throw Error(JSON.stringify(o));max=count=0;}
for(let i=0;i<128;i++){check(r.portamento(i));check(r.glideCoef(i));}finish('source_glide_times_and_44100_control_coefficients',3e-7);
for(const amount of [1,32,64,96,127])for(let dir=0;dir<2;dir++)for(let mode=1;mode<=4;mode++){
 const from=dir?84:36,to=dir?36:84,e=new r.Engine(44100,{maxVoices:4});e.setPatch({portamento:amount,osc3Mode:mode,osc3Volume:64,osc3Semitone:64,osc3Detune:64,osc2Detune:20,filter1Keyfollow:96,filterKeytrackBase:60,cutoff:50,saturationCurve:12,oscVolume:96});e.lastNote=from;e.noteOn(to,100);const v=e.voices.find(v=>v.active);
 for(let tick=0;tick<5000;tick++){if(tick)v.control();check(v.pitch);check(v.inc[0]);check(v.inc[1]);check(v.inc[2]);check(v.filter1.cutoff);check(v.saturation.coef);}
}
finish('source_glide_pitch_oscillators_filter_and_follow_saturation',3e-5);
if(at!==data.length)throw Error('length mismatch');console.log(JSON.stringify(results,null,2));
