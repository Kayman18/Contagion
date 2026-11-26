const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/lfo-timing'),{maxBuffer:4e6});let at=0,max=0,maxScaled=0,count=0;const results=[];
function check(x){const y=data.readFloatLE(at);at+=4;const d=Math.abs(x-y);if(!Number.isFinite(y))throw Error('nonfinite');if(d/Math.max(1,Math.abs(x))>.00005)throw Error(JSON.stringify({count,at,x,y}));max=Math.max(max,d);maxScaled=Math.max(maxScaled,d/Math.max(1,Math.abs(x)));count++;}
function finish(name,limit){const out={name,values:count,max_absolute_error:max,max_scaled_error:maxScaled,limit,pass:maxScaled<limit};results.push(out);if(!out.pass)throw Error(JSON.stringify(out));max=maxScaled=count=0;}
for(let note=0;note<128;note++)for(let track=0;track<128;track++)for(const rate of [0,64,127])check(r.lfoRate(rate)*r.lfoKeytrack(track,note)*Math.fround(1.02));
finish('all_notes_and_keytrack_values',2e-6);
for(let clock=1;clock<=21;clock++)for(let tempo=0;tempo<128;tempo++)for(let d=0;d<3;d++)check(1/r.lfoPeriod(clock,63+tempo));
finish('all_clock_divisions_and_tempos_bypass_keytrack_and_deviation',2e-6);
for(const [ci,clock] of [0,5,17].entries())for(const note of [36,84])for(let mode=1;mode<=2;mode++){
 Math.random=()=>.5;const e=new r.Engine(44100,{maxVoices:1});e.setPatch({lfo1Clock:clock,clockTempo:[0,57,127][ci],lfo1Keyfollow:96,lfo1Rate:96,lfo1Shape:0,lfo1Contour:64,lfo1TrigPhase:1,lfo1EnvMode:mode===2?1:0,lfo1Mode:0});e.updateEffects();e.noteOn(note,100);const v=e.voices.find(v=>v.active);v.lfoDeviation[0]=1;
 const l=new Float32Array(1),rr=new Float32Array(1);
 for(let i=0;i<8000;i++){
  if(i===4000){e.setPatch({...e.patch,clockTempo:100,lfo1Keyfollow:32});e.updateEffects();v.controlCountdown=0;}
  v.render(l,rr,0,1);check(v.lfos[0].rate);check(v.lfos[0].phase);check(v.lfos[0].value);
 }
}
finish('live_voice_timing_tempo_changes_keytrack_and_one_shot',.00005);
if(at!==data.length)throw Error('length mismatch');console.log(JSON.stringify(results,null,2));
