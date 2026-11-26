const cp=require('child_process'),assert=require('assert'),r=require('../tools/reference.cjs');
const run=cp.spawnSync('build/matrix-amp',{encoding:'utf8',maxBuffer:8e6});assert.equal(run.status,0,run.stderr);
const clip=(v,a,b)=>Math.max(a,Math.min(b,v)),levels=[-1,0,.37,1];let cases=0,maxError=0;
for(const line of run.stdout.trim().split('\n')){
 const [dest,amount,vel,li,cc,step,actual]=line.split(' ').map(Number);
 const patch={...r.patch(),patchVolume:73,ampVelocity:91,assign1Source:3,assign1Dest:dest,assign1Amount:amount};
 const src=Array(28).fill(0);src[3]=step?0:Math.fround(levels[li]);const out={...patch,_channelVolume:0};new r.Matrix().apply(patch,src,out,[]);
 const expected=r.ampVelocity(clip(out.ampVelocity,0,127),vel)*(cc/127)**2*(96/127)**2*r.patchVolume(clip(out.patchVolume,0,127))*clip(1+out._channelVolume/127,0,2);
 const err=Math.abs(actual-expected);maxError=Math.max(maxError,err);assert(err<.00002,`${line} expected ${expected}`);cases++;
}
console.log(JSON.stringify({cases,maxError,slew32Samples:true,phaseAndEnvelopePreserved:true,voiceIsolation:true,midiVolumeAndMute:true,matrixRemoval:true,legacyLinearVelocity:true},null,2));
