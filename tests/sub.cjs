const cp=require('child_process'),r=require('../tools/reference.cjs');
const data=cp.execFileSync('./build/sub'),report={samples:0,max_error:0};let at=0;
for(let shape=0;shape<2;shape++)for(let rate=0;rate<4;rate++)for(let phase=0;phase<4;phase++){
 const s=new r.Sub();s.reset(phase*.25);s.triangle=!!shape;
 for(let j=0;j<4096;j++){const x=s.next((1<<rate)/1024),y=data.readFloatLE(at);at+=4;if(!Number.isFinite(y))throw Error('nonfinite');report.max_error=Math.max(report.max_error,Math.abs(x-y));report.samples++;}
}
report.pass=report.max_error<1e-6&&at===data.length;if(!report.pass)throw Error(JSON.stringify(report));console.log(JSON.stringify(report,null,2));
