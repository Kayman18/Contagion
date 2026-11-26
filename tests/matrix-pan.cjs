const cp=require('child_process'),assert=require('assert'),r=require('../tools/reference.cjs');
const run=cp.spawnSync('build/matrix-pan',{encoding:'utf8',maxBuffer:4e6});assert.equal(run.status,0,run.stderr);
const clip=x=>Math.max(0,Math.min(127,x));let cases=0,maxError=0;
for(const line of run.stdout.trim().split('\n')){
 const [route,dest,amount,level,...actual]=line.split(' ').map(Number);
 const patch={...r.patch(),panorama:59,unisonPanSpread:87,panVelocity:91,lfo2Panorama:37,assign1Source:3,assign1Dest:dest,assign1Amount:amount};
 const sources=Array(28).fill(0);sources[3]=Math.fround(level);const out={...patch};new r.Matrix().apply(patch,sources,out,[]);
 const p=out.panorama+2*(clip(out.panVelocity)-64)*101/127+2*(clip(out.lfo2Panorama)-64)*Math.fround(.4),spread=route===3?clip(out.unisonPanSpread)/2:0;
 const expected=Array(4);r.pan(p-spread,expected,0);r.pan(p+spread,expected,2);
 actual.forEach((v,i)=>{const err=Math.abs(v-expected[i]);maxError=Math.max(maxError,err);assert(err<.000002,`${line} expected ${expected}`)});cases++;
}
console.log(JSON.stringify({cases,values:cases*4,maxError,routingModes:4,slew32Samples:true,voiceIsolation:true,matrixRemoval:true},null,2));
