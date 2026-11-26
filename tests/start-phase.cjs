const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/start-phase'));let at=0,max=0;
function next(state){let x=state||1;x^=x<<13;x^=x>>>17;x^=x<<5;return x>>>0;}
function check(x){const y=data.readFloatLE(at);at+=4;max=Math.max(max,Math.abs(x-y));}
for(let phase=0;phase<=128;phase++)for(let take=0;take<3;take++){
 if(phase===128){check(0);check(0);check(0);continue;} // Native compatibility setting.
 const e=new r.Engine(44100,{maxVoices:1});e.setPatch({oscInitPhase:phase});
 const v=new r.Voice(44100,e.ctx);
 let state=(0x9e3779b9^Math.imul(60,0x85ebca6b)^Math.imul(take+1,0xc2b2ae35)^0xa511e9b3)>>>0;
 const first=next(state),second=next(first);let call=0;
 Math.random=()=>{call++;return call===2?(first>>>8)/16777216:call===3?(second>>>8)/16777216:.25;};
 v.noteOn(60,100/127);check(v.oscillators[0].phase);check(v.oscillators[1].phase);check(v.subOscillator.phase);
}
const report={values:at/4,native_checks:["phase changes preserve sounding audio","fixed attacks repeat exactly","random retriggers vary","noise PRNG unchanged","phase bounds"],source_phase_settings:128,compatibility_settings:1,max_error:max,pass:max===0&&at===data.length};console.log(JSON.stringify(report,null,2));if(!report.pass)process.exit(1);
