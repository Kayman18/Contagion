// Compare native controls and oscillator-2 increments with the captured Voice.
const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/fm'),{maxBuffer:4e6});let at=0,max=0,count=0,squares=0;const results=[];
function check(x){const y=data.readFloatLE(at);at+=4;const d=Math.abs(x-y);if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,d);squares+=d*d;count++;}
function finish(name,limit){let result={name,values:count,max_error:max,rms_error:Math.sqrt(squares/count),limit,pass:max<limit};results.push(result);if(!result.pass)throw Error(JSON.stringify(result));max=count=squares=0;}
for(let amount=0;amount<128;amount++)for(const env of [0,64,127])for(const vel of [0,64,127])for(const level of [0,.5,1]){
 const engine=new r.Engine(44100,{maxVoices:1});engine.setPatch({fmAmount:amount,fmFiltEnvAmt:env,fmAmountVelocity:vel});engine.noteOn(60,100);const v=engine.voices.find(v=>v.active);v.velocity=Math.fround(100/127);v.filterEnvelope.level=level;v.control();check(v.fmDepth);
}
finish('source_fm_depth_envelope_velocity',3e-6);
for(let mode=0;mode<4;mode++)for(const amount of [0,1,32,64,96,127])for(const shape of [0,32,64,127])for(const color of [0,64,127]){
 const engine=new r.Engine(44100,{maxVoices:1});engine.setPatch({fmMode:mode,fmAmount:amount,osc1Shape:shape,noiseColor:color,noiseVolume:0,punchIntensity:0});engine.noteOn(60,100);
 const v=engine.voices.find(v=>v.active);v.control();v.control=()=>{};v.inc[0]=1/128;v.inc[1]=3/256;v.sync=false;v.noise.lp=0;let rng=0x12345678;
 Math.random=()=>{rng^=rng<<13;rng^=rng>>>17;rng^=rng<<5;rng>>>=0;return (rng>>>8)/16777216;};
 let n=0;v.oscillators[0].next=()=>((n%17)-8)/8;v.oscillators[1].next=inc=>{check(inc);return 0;};
 const l=new Float32Array(1),rr=new Float32Array(1);
 for(n=0;n<64;n++){v.oscillators[0].phase=n/64;v.render(l,rr,0,1);}
}
finish('actual_source_render_fm_increments_four_modes',3e-6);
if(at!==data.length)throw Error('count mismatch');console.log(JSON.stringify(results,null,2));
