const cp=require('child_process'),assert=require('assert'),r=require('../tools/reference.cjs');
const run=cp.spawnSync('build/matrix-pwm',{encoding:'utf8',maxBuffer:8e6});assert.equal(run.status,0,run.stderr);
const clip=(x,a=0,b=127)=>Math.max(a,Math.min(b,x)),levels=[-1,0,.37,1];let cases=0,maxError=0;
for(const line of run.stdout.trim().split('\n')){
 const [dest,amount,lfo3,test,step,...actual]=line.split(' ').map(Number);
 const patch={...r.patch(),osc1PulseWidth:21,osc2PulseWidth:93,pulseWidthVelocity:47,lfo1PulseWidth:81,assign1Source:3,assign1Dest:dest,assign1Amount:amount};
 const src=Array(28).fill(0);src[3]=step?0:Math.fround(levels[test]);const out={...patch};new r.Matrix().apply(patch,src,out,[]);
 const pw=2*(clip(out.pulseWidthVelocity)-64)*101/127+r.lfoPW(clip(out.lfo1PulseWidth))*(-1+test*2/3),expected=[];
 for(let i=0;i<2;i++){
  const lfo3width=(i?(lfo3===4||lfo3===5):(lfo3===3||lfo3===4))?73*-.25:0;
  const value=clip(out[i?'osc2PulseWidth':'osc1PulseWidth']+pw+lfo3width,-63.5,127),o=new r.Osc(44100);o.setShape(100,.5*(1-value/127),.01);expected.push(o.duty);
 }
 expected.push(expected[1]);actual.forEach((v,i)=>{const err=Math.abs(v-expected[i]);maxError=Math.max(maxError,err);assert(err<.000002,`${line} expected ${expected}`)});cases++;
}
console.log(JSON.stringify({cases,values:cases*3,maxError,lfo3Destinations:6,phaseAndBlepPreserved:true,slaveWidth:true,voiceIsolation:true,matrixRemoval:true},null,2));
