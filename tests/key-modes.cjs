const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs'),events=require('./key-modes-events.json');
const data=cp.execFileSync(path.join(__dirname,'../build/key-modes'));let at=0,max=0;
function check(x,field,i,mode){let y=data.readFloatLE(at);at+=4;let err=Math.abs(x-y);max=Math.max(max,err);if(!Number.isFinite(y)||err>(field===3?8e-5:field===4?2e-7:0))throw Error(JSON.stringify({mode,event:i,action:events[i],field,expected:x,actual:y,error:err}));}
for(let mode=1;mode<=4;mode++){
 const e=new r.Engine(44100,{maxVoices:4});e.setPatch({keyMode:mode,portamento:127});
 // Isolate allocation/retrigger/glide-start policy from DSP control stepping.
 for(const v of e.voices)v.control=()=>{};
 for(let i=0;i<events.length;i++){
  for(const v of e.voices)if(v.active)v.pitch+=(v.note-v.pitch)*.1;
  const [t,n,vel]=events[i];if(t===0)e.noteOn(n,vel);else if(t===1)e.noteOff(n);else if(t===2)e.controlChange(64,n);else if(t===3)e.allNotesOff();
  else if(t===4){for(const v of e.voices)if(v.active&&v.held){v.ampEnvelope.stage=v.filterEnvelope.stage=3;v.ampEnvelope.level=v.filterEnvelope.level=.4;}}
  else if(t===5){for(const v of e.voices)if(v.active&&!v.held)v.kill();}
  const active=e.voices.filter(v=>v.active),v=active[0];const values=[active.length,v?v.note:-1,v?+v.held:0,v?v.pitch:0,v?v.velocity:0,v?v.ampEnvelope.stage:0,v?v.filterEnvelope.stage:0,e.held.length,e.lastNote??-1,+e.holdPedal];values.forEach((x,k)=>check(x,k,i,mode));
 }
}
if(at!==data.length)throw Error('length mismatch');console.log(JSON.stringify({modes:4,events_per_mode:events.length,values:at/4,max_error:max,pass:true},null,2));
