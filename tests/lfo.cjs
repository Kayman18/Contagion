const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/lfo'),{maxBuffer:16e6});let at=0,max=0,count=0,squares=0;const results=[];
function check(x){let y=data.readFloatLE(at);at+=4;let d=Math.abs(x-y);if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,d);squares+=d*d;count++;}
function finish(name,limit){const v={name,values:count,max_error:max,rms_error:Math.sqrt(squares/count),limit,pass:max<limit};results.push(v);if(!v.pass)throw Error(JSON.stringify(v));max=count=squares=0;}
for(let shape=0;shape<6;shape++)for(let contour=0;contour<128;contour++)for(let env=0;env<2;env++)for(const trigger of [0,1,65]){
 let rng=0x12345678;Math.random=()=>{rng^=rng<<13;rng^=rng>>>17;rng^=rng<<5;rng>>>=0;return (rng>>>8)/16777216;};
 const l=new r.LFO(32768);l.phase=Math.floor(l.phase*65536)/65536;l.setRate(5.5);l.setShape(shape);l.setContour(contour-64);l.setEnvMode(!!env);l.trigger(trigger?(trigger-1)/128:null);
 for(let n=0;n<512;n++)check(l.tick(32));
 if(env&&!l.done)throw Error('one shot did not finish');
}
finish('six_source_lfo_shapes_all_contours_trigger_and_one_shot',.00005);
for(const note of [24,60,96])for(const amount of [0,32,64,80,96,112,127])for(const pw of [0,64,127])for(const value of [-1,-.25,0,.5,1,2]){
 const e=new r.Engine(44100,{maxVoices:1});e.setPatch({osc1PulseWidth:32,osc2PulseWidth:96,osc2Detune:20,lfo1Osc1:amount,lfo1Osc2:127-amount,lfo1PulseWidth:pw,osc2FiltEnvAmt:80});e.noteOn(note,100);
 const v=e.voices.find(v=>v.active);v.lfoValue=i=>i===0?value:0;v.filterEnvelope.level=.5;v.control();check(v.inc[0]);check(v.inc[1]);check(v.oscillators[0].duty);check(v.oscillators[1].duty);
}
finish('source_voice_lfo_pitch_and_pwm_routes',.000002);
for(let mode=1;mode<=2;mode++){
 Math.random=()=>.5;const e=new r.Engine(44100,{maxVoices:1});e.setPatch({lfo1Rate:96,lfo1Shape:0,lfo1Contour:64,lfo1TrigPhase:1,lfo1EnvMode:mode===2?1:0,lfo1Mode:0,lfo1Osc1:80,lfo1Osc2:96,lfo1PulseWidth:80,osc2Detune:20,ampEnvRelease:80});e.noteOn(60,100);
 const v=e.voices.find(v=>v.active),l=new Float32Array(1),rr=new Float32Array(1);
 for(let n=0;n<12000;n++){v.render(l,rr,0,1);check(v.inc[0]);check(v.inc[1]);check(v.oscillators[0].duty);check(v.oscillators[1].duty);check(v.lfos[0].value);}
}
finish('live_source_lfo_control_timing_and_destinations',.00005);
if(at!==data.length)throw Error('count mismatch');console.log(JSON.stringify(results,null,2));
