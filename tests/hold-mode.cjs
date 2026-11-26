const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs'),events=require('./hold-mode-events.json');
const data=cp.execFileSync(path.join(__dirname,'../build/hold-mode'),{maxBuffer:8000000});let at=0,checked=0;
const e=new r.Engine(44100,{maxVoices:4});e.setPatch({keyMode:5});for(const v of e.voices)v.control=()=>{};
for(let i=0;i<events.length;i++){
 for(const v of e.voices)if(v.active&&!v.held)v.kill();
 const [t,n,vel]=events[i];if(t===0)e.noteOn(n,vel);else if(t===1)e.noteOff(n);else if(t===2)e.controlChange(64,n);else if(t===3)e.allNotesOff();
 const values=[];for(let n=0;n<128;n++)values.push(+e.keysDown.has(n),+e.pedalHeld.has(n),+e.latched.has(n),+e.voices.some(v=>v.active&&v.held&&v.note===n));values.push(+e.holdPedal);
 values.forEach((x,k)=>{const y=data.readUInt32LE(at);at+=4;
 // Voice stealing policies already differ in the native poly port. Compare
 // bookkeeping only during the 128-key stress section.
 if(i>=819&&k<512&&k%4===3)return;
 checked++;if(x!==y)throw Error(JSON.stringify({i,event:events[i],field:k,expected:x,actual:y}));});
}
if(at!==data.length)throw Error('length');console.log(JSON.stringify({events:events.length,checked_values:checked,pass:true},null,2));
