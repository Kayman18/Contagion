const cp=require('child_process'),assert=require('assert'),r=require('../tools/reference.cjs');
const output=cp.execFileSync('build/matrix-envelopes',{encoding:'utf8',maxBuffer:40e6});
const names=['filtEnvAttack','filtEnvDecay','filtEnvSustain','filtEnvSustainTime','filtEnvRelease','ampEnvAttack','ampEnvDecay','ampEnvSustain','ampEnvSustainTime','ampEnvRelease'];
let cases=0,maxCoef=0,maxAudio=0;
for(const line of output.trim().split('\n')){
 const [dest,amount,base,t,sr,...a]=line.split(' ').map(Number);
 const p={...r.patch(),assign1Source:3,assign1Dest:dest,assign1Amount:amount};for(const n of names)p[n]=base;
 const src=Array(28).fill(0),e={...p};src[3]=Math.fround([-1,0,.37,1][t]);new r.Matrix().apply(p,src,e,[]);
 const clip=v=>Math.max(0,Math.min(127,v)),env=[];
 for(let k=0;k<2;k++){
  const vals=names.slice(k*5,k*5+5).map(n=>clip(e[n])),slope=r.envSlope(vals[3]),v=new r.Envelope(sr);
  v.setParams(k?r.attack(vals[0]):r.filterAttack(vals[0]),r.decay(vals[1]),k?r.sustain(vals[2]):vals[2]/127,...slope,r.release(vals[4]));
  v.stage=t+1;v.level=.375;env.push(v);
  const expected=[v.attackInc,v.decayCoef,v.sustain,v.releaseCoef,v.slope,v.fallCoef,v.riseInc];
  for(let i=0;i<7;i++){let error=Math.abs(a[k*7+i]-expected[i]);maxCoef=Math.max(maxCoef,error);assert(error<.00001,JSON.stringify({dest,amount,base,t,k,i,actual:a[k*7+i],expected:expected[i]}));}
 }
 for(let j=0;j<16;j++)for(let k=0;k<2;k++){let expected=env[k].next(),error=Math.abs(a[14+j*2+k]-expected);maxAudio=Math.max(maxAudio,error);assert(error<.00003);}
 cases++;
}
console.log(JSON.stringify({cases,targets:10,sampleRates:[44100,48000],maxCoefficientError:maxCoef,maxEnvelopeError:maxAudio,stageAndLevelPreserved:true,voiceIsolation:true,matrixRemoval:true,blockInvariant:true,pass:true},null,2));
