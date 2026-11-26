const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');const data=cp.execFileSync(path.join(__dirname,'../build/patch-volume'));let at=0,max=0;
function check(x){const y=data.readFloatLE(at);at+=4;max=Math.max(max,Math.abs(x-y));if(!Number.isFinite(y))throw Error('nonfinite');}
for(let a=0;a<128;a++)for(let c=0;c<4;c++)check(r.patchVolume(a)*(c*42/127)**2*(96/127)**2*(100/127));
for(let a=0;a<128;a++)for(let j=0;j<40;j++)check(1+(r.patchVolume(a)-1)*Math.min(1,(j+1)/32));
if(at!==data.length||max>3e-6)throw Error(JSON.stringify({at,length:data.length,max}));console.log(JSON.stringify({values:at/4,max_error:max,pass:true},null,2));
