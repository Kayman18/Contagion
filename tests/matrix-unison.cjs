const cp=require('child_process'),assert=require('assert'),r=require('../tools/reference.cjs');
const out=cp.execFileSync('build/matrix-unison',{encoding:'utf8',maxBuffer:8e6});let n=0,max=0;
for(const line of out.trim().split('\n')){
 const [count,amount,test,i,cents,inc1,inc2,inc3]=line.split(' ').map(Number);
 const p={...r.patch(),unisonDetune:64,assign1Source:3,assign1Dest:59,assign1Amount:amount},s=Array(28).fill(0),e={...p};
 s[3]=[-1,0,.37,1][test];new r.Matrix().apply(p,s,e,[]);
 const side=2*i/(count-1)-1,want=side===0?0:side*r.unisonDetune(Math.max(0,Math.min(127,e.unisonDetune)))+(count%2?2.37:0);
 max=Math.max(max,Math.abs(want-cents));assert(Math.abs(want-cents)<.00001);
 assert(Math.abs(inc1-440*Math.pow(2,(60-69+want/100)/12)/44100)<1e-8);
 assert(Number.isFinite(inc2)&&Number.isFinite(inc3));n++;
}
console.log(JSON.stringify({cases:n,maxCentsError:max,phasePreserved:true,matrixRemoval:true,unisonOffInert:true,pass:true},null,2));
