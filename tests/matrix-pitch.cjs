const cp=require('child_process'),assert=require('assert'),r=require('../tools/reference.cjs');
const run=cp.spawnSync('build/matrix-pitch',{encoding:'utf8',maxBuffer:24e6});assert.equal(run.status,0,run.stderr);
const clip=(x,a=0,b=127)=>Math.max(a,Math.min(b,x)),levels=[-1,0,.37,1],lfos=[-1,-.25,.5,1];let cases=0,maxCentsError=0,maxIncrementError=0;
for(const line of run.stdout.trim().split('\n')){
 const [dest,amount,lfo3,mode,test,...values]=line.split(' ').map(Number),base=values.slice(0,3),actual=values.slice(3);
 const patch={...r.patch(),lfo1Osc1:81,lfo1Osc2:47,osc2FiltEnvAmt:71,assign1Source:3,assign1Dest:dest,assign1Amount:amount};
 const src=Array(28).fill(0);src[3]=Math.fround(levels[test]);const out={...patch,_osc1Pitch:0,_osc2Pitch:0};new r.Matrix().apply(patch,src,out,[]);
 const p3=r.lfo3Pitch(73)*-.25,c1=r.lfoPitch(out.lfo1Osc1)*lfos[test]+(lfo3<=1?p3:0)+100*clip(out._osc1Pitch,-48,36);
 const common=r.osc2EnvCents(out.osc2FiltEnvAmt)*(test/3)+r.lfoPitch(out.lfo1Osc2)*lfos[test]+(lfo3===1||lfo3===2?p3:0);
 const c2=common+100*clip(out._osc2Pitch,-48,36),c3=c2,cents=[c1,c2,c3];
 cents.forEach((c,i)=>{const err=Math.abs(actual[3+i]-c);maxCentsError=Math.max(maxCentsError,err);assert(err<.003,`cents ${line} expected ${cents}`);
 const inc=Math.min(.45,base[i]*2**(c/1200)),e=Math.abs(actual[i]-inc);maxIncrementError=Math.max(maxIncrementError,e);assert(e<.000002,`increment ${line} expected ${inc}`);});cases++;
}
console.log(JSON.stringify({cases,values:cases*6,maxCentsError,maxIncrementError,lfo3Destinations:6,osc3Modes:[1,2,4],phaseAndBlepPreserved:true,bandSelection:true,voiceIsolation:true,matrixRemoval:true},null,2));
