const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/pitch-env'),{maxBuffer:4e6});let at=0,max=0,count=0,squares=0;const result=[];
function check(x){let y=data.readFloatLE(at);at+=4;let d=Math.abs(x-y);if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,d);squares+=d*d;count++;}
function finish(name,limit){let v={name,values:count,max_error:max,rms_error:Math.sqrt(squares/count),limit,pass:max<limit};result.push(v);if(!v.pass)throw Error(JSON.stringify(v));max=count=squares=0;}
for(const note of [0,24,60,96,127])for(let amount=0;amount<128;amount++)for(const level of [0,.25,.5,1])for(const bend of [-.5,0,.5]){
 const e=new r.Engine(44100,{maxVoices:1});e.setPatch({osc2FiltEnvAmt:amount,osc2Detune:20,osc2Semitone:64,osc2Keyfollow:96,bendScale:0,bendUp:66,bendDown:62});e.pitchBend(bend);e.noteOn(note,100);
 const v=e.voices.find(v=>v.active);v.filterEnvelope.level=level;v.control();check(v.inc[1]);check(v.oscillators[1].tables.indexOf(v.oscillators[1].table));
}
finish('oscillator2_pitch_envelope_all_amounts_notes_bends_bands',.000002);
for(const amount of [0,80,127]){
 const e=new r.Engine(44100,{maxVoices:1});e.setPatch({osc2FiltEnvAmt:amount,osc2Detune:20,filtEnvAttack:12,filtEnvDecay:55,filtEnvSustain:32,filtEnvRelease:44,ampEnvRelease:80,punchIntensity:0});e.noteOn(60,100);
 const v=e.voices.find(v=>v.active),l=new Float32Array(1),rr=new Float32Array(1);
 for(let n=0;n<6000;n++){if(n===3000)e.noteOff(60);v.render(l,rr,0,1);check(v.filterEnvelope.level);check(v.inc[1]);}
}
finish('live_pitch_envelope_control_cadence_and_release',.00005);
if(at!==data.length)throw Error('count mismatch');console.log(JSON.stringify(result,null,2));
