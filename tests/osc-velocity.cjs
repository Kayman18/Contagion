const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/osc-velocity'),{maxBuffer:20e6});let at=0,max=0,count=0;
function check(x){const y=data.readFloatLE(at);at+=4;if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,Math.abs(x-y));count++;}
for(let route=0;route<4;route++)for(let mode=1;mode<=4;mode++)for(let amount=0;amount<128;amount++)for(const velocity of [1,32,64,100,127]){
 const e=new r.Engine(44100,{maxVoices:4});e.setPatch({filterRouting:route,filterBalance:64,osc3Mode:mode,osc3Volume:64,osc3Semitone:64,osc3Detune:64,osc1Shape:16,osc2Shape:96,osc1PulseWidth:40,osc2PulseWidth:80,osc1ShapeVelocity:amount,osc2ShapeVelocity:127-amount,pulseWidthVelocity:amount,panVelocity:127-amount,lfo1PulseWidth:88,lfo2Shape12:96,lfo2Panorama:96,osc2Detune:20,cutoff:73,cutoff2:91,filter2CutoffLink:0,filter1Resonance:55,filter2Resonance:70,panorama:64,unisonPanSpread:64,punchIntensity:0});e.noteOn(60,velocity);
 const v=e.voices.find(v=>v.active),orig=v.control.bind(v);v.control=()=>{};let a=0,b=0;v.oscillators[0].next=()=>((a++%31)-15)/128;v.oscillators[1].next=()=>((b++%23)-11)/128;v.punchAt=1e9;v.ampEnvelope.next=()=>1;v.filterEnvelope.next=()=>0;
 for(let step=0;step<2;step++){
  const value=[-.25,.5][step];v.lfoValue=i=>i<2?value:0;orig();
  for(const o of v.oscillators){check(o.waveGain);check(o.sawGain);check(o.pulseGain);check(o.duty);}
  const p=64+2*(127-amount-64)*velocity/127+64*value,d=route===3?32:0,expected=new Float64Array(4);r.pan(p-d,expected,0);r.pan(p+d,expected,2);
  const factor=(v.gainTarget[0]+v.gainTarget[1])/(expected[0]+expected[1]);for(let i=0;i<4;i++)v.gainTarget[i]/=factor;if(!step)v.gainNow.set(v.gainTarget);
  Object.assign(v.gain,{osc1:1,osc2:1,osc3:0,noise:0,sub:0,ring:0,filter:1});v.fmDepth=0;v.controlCountdown=32;
  const l=new Float32Array(1),rr=new Float32Array(1);for(let n=0;n<32;n++){l[0]=rr[0]=0;v.render(l,rr,0,1);check(l[0]);check(rr[0]);for(let i=0;i<4;i++)check(v.gainNow[i]);}
 }
}
const report={values:count,max_error:max,limit:3e-5,pass:max<3e-5};if(at!==data.length||!report.pass)throw Error(JSON.stringify(report));console.log(JSON.stringify(report,null,2));
