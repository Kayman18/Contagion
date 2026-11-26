const cp=require('child_process'),r=require('../tools/reference.cjs');
const data=cp.execFileSync('build/lfo-waves',{maxBuffer:24000000});let at=0,max=0;
function check(x,exact=false){const y=data.readFloatLE(at);at+=4;const err=Math.abs(x-y);max=Math.max(max,err);if(!Number.isFinite(y)||err>(exact?0:0.00008))throw Error(JSON.stringify({at,expected:x,actual:y,err}));}
for(let shape=6;shape<=67;shape++)for(const x of r.tables(shape-4)[0])check(x,true);
for(let shape=6;shape<=67;shape++)for(let contour=-64;contour<=63;contour++){
 const l=new r.LFO(32768);l.setShape(shape);l.setContour(contour);
 for(let k=0;k<=512;k++)check(l.shapeAt(k/512));
}
for(let shape=6;shape<=67;shape++)for(let env=0;env<=1;env++){
 const l=new r.LFO(32768);l.setShape(shape);l.setContour(48);l.setRate(5.5);l.setEnvMode(!!env);l.trigger(.5);
 for(let k=0;k<512;k++)check(l.tick(32));
}
if(at!==data.length)throw Error('length');console.log(JSON.stringify({shapes:62,exact_table_samples:62*2049,values:at/4,max_error:max,pass:true},null,2));
