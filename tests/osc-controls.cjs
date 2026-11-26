const cp=require('child_process'),r=require('../tools/reference.cjs');
const data=cp.execFileSync('./build/osc-controls',{maxBuffer:1e6});let at=0,max=0,sum=0,count=0;
function check(x){const y=data.readFloatLE(at);at+=4;const d=Math.abs(x-y);if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,d);sum+=d*d;count++;}
for(const note of [0,36,60,84,127])for(const tune of [0,16,52,64,71,112,127])for(const follow of [0,32,64,80,96,112,127])for(const width of [0,32,96,127])for(const bend of [-1,0,.5]){
 const e=new r.Engine(44100,{maxVoices:4});
 // Native wrapper uses linear +/-2-semitone bend; disable source quadratic scaling.
 e.setPatch({osc1Semitone:tune,osc2Semitone:127-tune,osc1Keyfollow:follow,osc2Keyfollow:127-follow,osc1PulseWidth:width,osc2PulseWidth:127-width,osc2Detune:20,bendScale:0});
 e.pitchBend(bend);e.noteOn(note,100);let v=e.voices.find(v=>v.active);v.control();
 check(v.inc[0]);check(v.inc[1]);check(v.oscillators[0].duty);check(v.oscillators[1].duty);
}
const report={values:count,max_error:max,rms_error:Math.sqrt(sum/count),limit:2e-6,pass:max<2e-6&&at===data.length};console.log(JSON.stringify(report,null,2));if(!report.pass)process.exit(1);
