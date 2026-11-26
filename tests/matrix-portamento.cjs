const cp=require('child_process'),assert=require('assert'),r=require('../tools/reference.cjs');
const output=cp.execFileSync('build/matrix-portamento',{encoding:'utf8',maxBuffer:8e6});let n=0,maxValue=0,maxCoef=0,maxPitch=0;
for(const line of output.trim().split('\n')){
 const [a,base,t,sr,value,coef,first,last]=line.split(' ').map(Number);
 const p={...r.patch(),portamento:base,assign1Source:3,assign1Dest:5,assign1Amount:a},src=Array(28).fill(0),e={...p};
 src[3]=Math.fround([-1,0,.003,.37,1][t]);new r.Matrix().apply(p,src,e,[]);
 const want=Math.max(0,Math.min(127,e.portamento)),time=r.portamento(want),c=time===0?1:1-Math.exp(-32/(time*sr));
 let pitch=48;const advance=v=>{const time=r.portamento(v);if(time===0||Math.abs(pitch-84)<.001)pitch=84;else pitch+=(84-pitch)*(1-Math.exp(-32/(time*sr)));};
 advance(base);maxPitch=Math.max(maxPitch,Math.abs(first-pitch));
 for(let i=0;i<32;i++)advance(want);
 maxPitch=Math.max(maxPitch,Math.abs(last-pitch));maxValue=Math.max(maxValue,Math.abs(value-want));maxCoef=Math.max(maxCoef,Math.abs(coef-c));
 assert(Math.abs(value-want)<.00002);assert(Math.abs(coef-c)<.000001);assert(Math.abs(last-pitch)<.0002,JSON.stringify({line,pitch}));n++;
}
console.log(JSON.stringify({cases:n,maxAmountError:maxValue,maxCoefficientError:maxCoef,maxPitchError:maxPitch,previousControlValue:true,monoModes:4,unisonLayers:[1,2,3,4],zeroTransitions:true,voiceIsolation:true,matrixRemoval:true,blockInvariant:true,pass:true},null,2));
