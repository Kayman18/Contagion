const cp=require('child_process'),path=require('path'),r=require('../tools/reference.cjs');
const data=cp.execFileSync(path.join(__dirname,'../build/fm-osc'));let at=0,max=0;
for(const wave of [0,3])for(const shape of [0,32,64,127])for(const width of [.25,.5])for(let sync=0;sync<2;sync++){
 const o=new r.Osc(44100);o.reset(0);o.setWave(wave);o.setShape(shape,width,1/128);
 const steps=[-.25,-.0078125,0,.001953125,.125,.75,2.25];
 for(let n=0;n<512;n++){let x=o.next(steps[n%7],sync&&n%11===0?.375:-1),y=data.readFloatLE(at);at+=4;max=Math.max(max,Math.abs(x-y));if(!Number.isFinite(y))throw Error('nonfinite');}
}
const report={samples:at/4,max_error:max,limit:.000002,pass:max<.000002&&at===data.length};console.log(JSON.stringify(report,null,2));if(!report.pass)process.exit(1);
