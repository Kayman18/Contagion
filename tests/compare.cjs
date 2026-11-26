const fs=require('fs'),cp=require('child_process'),path=require('path');const r=require('../tools/reference.cjs');
const native=cp.execFileSync(path.join(__dirname,'../build/components'),{maxBuffer:4e6});let at=0,max=0,rms=0,count=0;const report=[];
function check(x){let y=native.readFloatLE(at);at+=4;let d=Math.abs(x-y);if(!Number.isFinite(y))throw Error('nonfinite');max=Math.max(max,d);rms+=d*d;count++;}
function end(name,tol){report.push({component:name,samples:count,max_error:max,rms_error:Math.sqrt(rms/count),limit:tol,pass:max<tol});if(max>=tol)throw Error(JSON.stringify(report.at(-1)));max=rms=count=0;}
for(let w=0;w<2;w++)for(let shape of [0,32,64,127]){let o=new r.Osc(44100);o.reset(0);o.setWave(w);o.setShape(shape,.5,.0078125);for(let i=0;i<2048;i++)check(o.next(.0078125));}end('oscillators',2e-6);
{let a=new r.Osc(44100),b=new r.Osc(44100);a.reset(0);b.reset(0);a.setShape(64,.5,.015625);b.setShape(127,Math.fround(.37),.0234375);for(let i=0;i<2048;i++){let sync=a.phase+.015625>=1?(1-a.phase)/.015625:-1;a.next(.015625);check(b.next(.0234375,sync));}}end('hard_sync',2e-6);
for(let mode=0;mode<4;mode++)for(let poles of [2,4])for(let res of [0,127]){let f=new r.Filter(44100);f.setMode(mode);f.setPoles(poles);f.set(80,res);for(let i=0;i<4096;i++)check(f.process(i===0?1:0));}end('dual_filter_component_modes',1e-4);
{let e=new r.Envelope(44100);e.setParams(Math.fround(.01),Math.fround(.04),.5,0,0,Math.fround(.03));e.trigger();for(let i=0;i<20000;i++){if(i===10000)e.release();check(e.next());}}end('ADSR_fixed_sustain',1e-4);
if(at!==native.length)throw Error('sample count mismatch');console.log(JSON.stringify(report,null,2));
