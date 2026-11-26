const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/midi-volume'));let at=0,max=0,count=0;
const engine=new r.Engine(44100,{maxVoices:4});
function check(x){const y=data.readFloatLE(at);at+=4;max=Math.max(max,Math.abs(x-y));count++;if(!Number.isFinite(y))throw Error('nonfinite');}
for(let a=0;a<128;a++)for(let b=0;b<128;b++){engine.controlChange(7,a);engine.controlChange(11,b);check(engine.ctx.volume);}
const curveMax=max;max=0;
engine.controlChange(7,64);engine.controlChange(11,96);
for(let i=0;i<4;i++)for(let j=0;j<32;j++)check(1+(engine.ctx.volume-1)*(j+1)/32);
if(at!==data.length||curveMax>3e-7||max>2e-6)throw Error(JSON.stringify({at,length:data.length,curveMax,max}));
console.log(JSON.stringify({values:count,source_curve_max_error:curveMax,native_ramp_max_error:max,pass:true},null,2));
