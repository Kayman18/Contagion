const cp=require('child_process'),fs=require('fs'),assert=require('assert'),ref=require('../tools/reference.cjs');
const presets=JSON.parse(fs.readFileSync('presets/factory-decoded.json'));
const result=cp.spawnSync('build/matrix-fm',{encoding:'utf8',maxBuffer:4e6});
if(result.status!==0)throw Error(result.stderr);
let maxError=0,count=0;
const clip=(v,a,b)=>Math.min(b,Math.max(a,v));
for(const line of result.stdout.trim().split('\n')){
 const [p,test,...actual]=line.split(' ').map(Number),patch={...(p<650?presets[p].patch:ref.patch()),cutoff:61,cutoff2:73,filter1Resonance:27,filter2Resonance:43,
 filter1EnvAmt:43,filter2EnvAmt:91,filter1Keyfollow:27,filter2Keyfollow:99,
 filter1EnvAmtVelocity:47,filter2EnvAmtVelocity:79,resonance1Velocity:43,resonance2Velocity:99,lfo1Resonance:64,lfo1FilterGain:64,lfo2Cutoff1:64,lfo2Cutoff2:64,fmAmount:63,fmFiltEnvAmt:43,lfo2FmAmount:93,fmAmountVelocity:81};
 if(p>=650){patch.assign1Source=3;patch.assign1Dest=[16,18,53,75][Math.floor((p-650)/128)];patch.assign1Amount=(p-650)%128;}
 const s=Array(28).fill(0),cc=[1,2,3,4,6,8,9,11,12,13,14,15,16,64,65,66];
 cc.forEach((c,i)=>s[ref.controllerSources[c]]=((i*11+test*29)%128)/127);
 s[1]=-.75+test*.5;s[2]=(31+test*21)/127;s[19]=.3;s[20]=test/3;s[21]=-.6;s[22]=-1+test*2/3;s[23]=-.25;
 s[24]=101/127;s[25]=.2;s[26]=((test<2?48:84)-60)/64;
 const out={...patch,_osc1Pitch:0,_osc2Pitch:0,_channelVolume:0};new ref.Matrix().apply(patch,s,out,[]);
 const bipolar=x=>2*(clip(x,0,127)-64);
 const expected=ref.fmDepth(clip(out.fmAmount+bipolar(out.fmFiltEnvAmt)*s[20]+bipolar(out.lfo2FmAmount)*s[22]+bipolar(out.fmAmountVelocity)*s[24],0,127));
 const err=Math.abs(actual[0]-expected);maxError=Math.max(maxError,err);assert(err<.00002,`preset ${p} case ${test}: ${actual[0]} != ${expected}`);count++;
}
console.log(JSON.stringify({presets:650,syntheticPatches:512,cases:count,maxError,releaseVelocityChecks:'passed'},null,2));
