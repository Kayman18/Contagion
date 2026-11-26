// Uses the actual captured voice's render/control methods, with deterministic
// test signals replacing its oscillators and envelope output fixed to unity.
const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/filter-routing'),{maxBuffer:8e6});
let at=0,max=0,sum=0,count=0,report=[];
function check(x){let y=data.readFloatLE(at);at+=4;let d=Math.abs(x-y);if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,d);sum+=d*d;count++;}
function finish(component,limit){const result={component,samples:count,max_error:max,rms_error:Math.sqrt(sum/count),limit,pass:max<limit};report.push(result);if(!result.pass)throw Error(JSON.stringify(result));max=sum=count=0;}
for(let sources=0;sources<4;sources++)for(let route=0;route<4;route++)for(let balance of [0,32,64,96,127])for(let mode=0;mode<4;mode++){
 const engine=new r.Engine(44100,{maxVoices:4});engine.setPatch({filterRouting:route,filterBalance:balance,filter1Mode:mode,filter2Mode:(mode+1)%4,cutoff:73,cutoff2:91,filter2CutoffLink:0,filter1Resonance:55,filter2Resonance:70,panorama:37,unisonPanSpread:86,punchIntensity:0});
 engine.noteOn(60,127);let v=engine.voices.find(v=>v.active);v.control();v.control=()=>{};v.controlCountdown=32;
 let i=0,j=0;v.oscillators[0].next=()=>{let n=i++;return n===0?.75:(n%31-15)/128;};v.oscillators[1].next=()=>{let n=j++;return n===7?-.5:(n%23-11)/128;};
 Object.assign(v.gain,{osc1:1,osc2:1,osc3:0,noise:0,ring:0,sub:0,filter:1});v.gain.sub=sources?r.sourceGain(93):0;v.gain.ring=sources?3*r.sourceGain(79):0;v.subOscillator.reset(0);v.subOscillator.triangle=sources===2;v.subInc=1/128;v.fmDepth=0;v.sync=false;v.punchAt=1e9;
 v.ampEnvelope.next=()=>1;v.filterEnvelope.next=()=>0;
 let pan=new Float64Array(4),d=route===3?43:0;r.pan(37-d,pan,0);r.pan(37+d,pan,2);v.gainNow.set(pan);v.gainTarget.set(pan);
 v.gain.noise=sources===3?r.sourceGain(80):0;v.noise.setColor(20);let rng=0x12345678;Math.random=()=>{rng^=rng<<13;rng^=rng>>>17;rng^=rng<<5;rng>>>=0;return (rng>>>8)/16777216;};
 let l=new Float32Array(1024),rr=new Float32Array(1024);v.render(l,rr,0,1024);
 for(let k=0;k<1024;k++){check(l[k]);check(rr[k]);}
}
finish('actual_source_voice_routing_stereo_sub_ring_and_noise',2e-5);
for(let slope=0;slope<128;slope++){
 let e=new r.Envelope(44100),s=r.envSlope(slope);e.setParams(Math.fround(.005),Math.fround(.001),Math.fround(.4),s[0],s[1],Math.fround(.015));e.trigger();
 for(let i=0;i<6000;i++){if(i===4000)e.release();check(e.next());}
}
finish('all_128_sustain_slope_settings',2e-4);
for(let variant=0;variant<4;variant++){
 const engine=new r.Engine(44100,{maxVoices:4}),note=48+variant*8;
 engine.setPatch({cutoff:47,cutoff2:67,filtEnvAttack:12,filtEnvDecay:55,filtEnvSustain:32,filtEnvRelease:44,filter1EnvAmt:70,filter2EnvAmt:81,filter1EnvPolarity:variant&1,filter2EnvPolarity:!(variant&1)?1:0,filter2CutoffLink:variant>>1,filter1Keyfollow:90,filter2Keyfollow:33,filterKeytrackBase:48,ampEnvRelease:40,punchIntensity:0});
 engine.noteOn(note,100);const v=engine.voices.find(v=>v.active);let l=new Float32Array(1),rr=new Float32Array(1);
 for(let j=0;j<6000;j++){
  if(j===3000)engine.noteOff(note);l.fill(0);rr.fill(0);v.render(l,rr,0,1);
  check(v.filterEnvelope.level);check(v.filter1.g/(1+v.filter1.g));check(v.filter2.g/(1+v.filter2.g));
 }
}
finish('filter_envelope_polarity_link_keytracking_and_control_timing',5e-5);
if(at!==data.length)throw Error('sample count mismatch');console.log(JSON.stringify(report,null,2));
