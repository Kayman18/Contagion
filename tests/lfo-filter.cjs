const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/lfo-filter'),{maxBuffer:16e6});let at=0,max=0,count=0,sum=0;
function check(x){const y=data.readFloatLE(at);at+=4;const d=Math.abs(x-y);if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,d);sum+=d*d;count++;}
for(let route=0;route<4;route++)for(let mode=0;mode<4;mode++)for(let amount=0;amount<128;amount++)for(const value of [-1,-.25,0,.5,1,2]){
 const e=new r.Engine(44100,{maxVoices:1});e.setPatch({filterRouting:route,filterBalance:64,filter1Mode:mode,filter2Mode:(mode+1)%4,cutoff:73,cutoff2:91,filter2CutoffLink:0,filter1Resonance:55,filter2Resonance:70,lfo1Resonance:amount,lfo1FilterGain:amount,panorama:64,unisonPanSpread:64,punchIntensity:0});e.noteOn(60,127);
 const v=e.voices.find(v=>v.active);v.lfoValue=i=>i===0?value:0;v.control();
 check(v.filter1.resonance);check(v.filter2.resonance);check(v.filter1.k);check(v.filter2.k);check(v.gain.filter);
 v.control=()=>{};v.controlCountdown=32;let a=0,b=0;v.oscillators[0].next=()=>((a++%31)-15)/128;v.oscillators[1].next=()=>((b++%23)-11)/128;
 Object.assign(v.gain,{osc1:1,osc2:1,osc3:0,noise:0,sub:0,ring:0});v.fmDepth=0;v.punchAt=1e9;v.ampEnvelope.next=()=>1;v.filterEnvelope.next=()=>0;
 let pan=new Float64Array(4),d=route===3?32:0;r.pan(64-d,pan,0);r.pan(64+d,pan,2);v.gainNow.set(pan);v.gainTarget.set(pan);
 const l=new Float32Array(64),rr=new Float32Array(64);v.render(l,rr,0,64);for(let n=0;n<64;n++){check(l[n]);check(rr[n]);}
}
const report={values:count,max_error:max,rms_error:Math.sqrt(sum/count),limit:2e-5,pass:max<2e-5};if(at!==data.length||!report.pass)throw Error(JSON.stringify(report));console.log(JSON.stringify(report,null,2));
