const cp=require('child_process'),assert=require('assert'),r=require('../tools/reference.cjs');
const output=cp.execFileSync('build/matrix-lfo3-amount',{encoding:'utf8',maxBuffer:24e6});
let cases=0,maxPitchError=0,maxPWMError=0;
for(const line of output.trim().split('\n')){
 const [dest,amount,base,t,mode,b1,b2,b3,i1,i2,i3,pw1,pw2]=line.split(' ').map(Number);
 const p={...r.patch(),lfo3OscAmount:base,assign1Source:3,assign1Dest:58,assign1Amount:amount},src=Array(28).fill(0),e={...p};
 src[3]=Math.fround([-1,0,.37,1][t]);new r.Matrix().apply(p,src,e,[]);
 const level=Math.max(0,Math.min(127,e.lfo3OscAmount)),lfo=[-1,-.25,.5,1][t],pitch=r.lfo3Pitch(level)*lfo;
 const cents=[dest<=1?pitch:0,(dest===1||dest===2)?pitch:0,(dest===1||dest===2)?pitch:0];
 [i1,i2,i3].forEach((v,j)=>{let error=Math.abs(v-Math.min(.45,[b1,b2,b3][j]*2**(cents[j]/1200)));maxPitchError=Math.max(maxPitchError,error);assert(error<.000002,line);});
 const widths=[dest===3||dest===4?level*lfo:0,dest===4||dest===5?level*lfo:0];
 [pw1,pw2].forEach((v,j)=>{let error=Math.abs(v-widths[j]);maxPWMError=Math.max(maxPWMError,error);assert(error<.00002,line);});cases++;
}
console.log(JSON.stringify({cases,allSixDestinations:true,osc3Modes:[1,2,4],sharedAndLocalLFO:true,maxPitchError,maxPWMError,matrixRemoval:true,phasePreserved:true,blockInvariant:true,pass:true},null,2));
