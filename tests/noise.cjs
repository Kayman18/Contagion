const cp=require('child_process'),r=require('../tools/reference.cjs');
const data=cp.execFileSync('./build/noise',{maxBuffer:8e6});let at=0,max=0,sum=0,count=0;
function seed(value){let state=value>>>0;Math.random=()=>{state^=state<<13;state^=state>>>17;state^=state<<5;state>>>=0;return (state>>>8)/16777216;};}
function check(x){let y=data.readFloatLE(at);at+=4;const d=Math.abs(x-y);if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,d);sum+=d*d;count++;}
for(let sr of [22050,44100,48000])for(let color=0;color<128;color++){
 seed(0x12345678);const n=new r.Noise(sr);n.setColor(color);for(let i=0;i<2048;i++)check(n.next());
}
seed(0xabcdef01);const n=new r.Noise(44100);
for(let i=0;i<65536;i++){n.setColor(Math.floor(i/257)%128);check(n.next());}
const result={samples:count,max_error:max,rms_error:Math.sqrt(sum/count),limit:3e-6,pass:max<3e-6&&at===data.length};console.log(JSON.stringify(result,null,2));if(!result.pass)process.exit(1);
