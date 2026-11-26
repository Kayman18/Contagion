const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/waves'),{maxBuffer:32e6});
let at=0,max=0,squares=0;
const waveCount=process.env.ALL_WAVES?64:4;
for(let w=0;w<waveCount;w++)for(const h of [128,96,64,48,32,24,16,12,8,6,4,3,2,1])for(const shape of [0,32,63])for(let sync=0;sync<2;sync++){
 const o=new r.Osc(44100),inc=Math.floor(Math.fround(Math.fround(.49)/h)*1048576)/1048576;o.reset(0);o.setWave(w);o.setShape(shape,Math.fround(.31),inc);
 for(let n=0;n<1024;n++){const x=o.next(inc,sync&&n%31===0?.375:-1),y=data.readFloatLE(at);at+=4;const d=Math.abs(x-y);if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,d);squares+=d*d;}
}
const report={waves:waveCount,pitch_bands:14,samples:at/4,max_error:max,rms_error:Math.sqrt(squares/(at/4)),limit:.000002,pass:max<.000002&&at===data.length};
console.log(JSON.stringify(report,null,2));if(!report.pass)process.exit(1);
