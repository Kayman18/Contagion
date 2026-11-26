const cp=require('child_process'),fs=require('fs'),assert=require('assert'),ref=require('../tools/reference.cjs');
const presets=JSON.parse(fs.readFileSync('presets/factory-decoded.json'));
const result=cp.spawnSync('build/matrix-filters',{encoding:'utf8',maxBuffer:4e6});
if(result.status!==0)throw Error(result.stderr);
let maxError=0,count=0;
const clip=(v,a,b)=>Math.min(b,Math.max(a,v));
for(const line of result.stdout.trim().split('\n')){
 const [p,test,...actual]=line.split(' ').map(Number),patch={...(p<650?presets[p].patch:ref.patch()),cutoff:61,cutoff2:73,filter1Resonance:27,filter2Resonance:43,
 filter1EnvAmt:43,filter2EnvAmt:91,filter1Keyfollow:27,filter2Keyfollow:99,
 filter1EnvAmtVelocity:47,filter2EnvAmtVelocity:79,resonance1Velocity:43,resonance2Velocity:99,lfo1Resonance:64,lfo1FilterGain:64,lfo2Cutoff1:64,lfo2Cutoff2:64};
 if(p>=650){patch.assign1Source=3;patch.assign1Dest=[28,29,30,31,76,77,78,79,48,49,54,55][Math.floor((p-650)/128)];patch.assign1Amount=(p-650)%128;}
 const s=Array(28).fill(0),cc=[1,2,3,4,6,8,9,11,12,13,14,15,16,64,65,66];
 cc.forEach((c,i)=>s[ref.controllerSources[c]]=((i*11+test*29)%128)/127);
 s[1]=-.75+test*.5;s[2]=(31+test*21)/127;s[19]=.3;s[20]=.7;s[21]=-.6;s[22]=.4;s[23]=-.25;
 s[24]=101/127;s[25]=.2;s[26]=((test<2?48:84)-60)/64;
 const out={...patch,_osc1Pitch:0,_osc2Pitch:0,_channelVolume:0};new ref.Matrix().apply(patch,s,out,[]);
 const vel=(amount)=>2*(clip(amount,0,127)-64)*101/127;
 const polarity=test>>1&1,key=(test<2?48:84)-60;
 const e1=ref.envAmount(out.filter1EnvAmt+vel(out.filter1EnvAmtVelocity))*.7*(polarity?1:-1);
 const e2=ref.envAmount(out.filter2EnvAmt+vel(out.filter2EnvAmtVelocity))*.7*(polarity?-1:1);
 const expected=[clip(out.cutoff+e1+ref.keyfollow(out.filter1Keyfollow,key)+ref.lfoResonance(out.lfo2Cutoff1)*.4,0,128),
 clip((test&1?out.cutoff+ref.linkedCutoff(out.cutoff2):out.cutoff2)+e2+ref.keyfollow(out.filter2Keyfollow,key)+ref.lfoResonance(out.lfo2Cutoff2)*.4,0,128),
 clip(out.filter1Resonance+vel(out.resonance1Velocity)+ref.lfoResonance(out.lfo1Resonance)*-.6,0,127),clip(out.filter2Resonance+vel(out.resonance2Velocity)+ref.lfoResonance(out.lfo1Resonance)*-.6,0,127),Math.max(0,1+ref.lfoFilterGain(clip(out.lfo1FilterGain,0,127))*-.6)];
 for(let i=0;i<5;i++){const err=Math.abs(actual[i]-expected[i]);maxError=Math.max(maxError,err);assert(err<.0001,`preset ${p} case ${test} dest ${i}: ${actual[i]} != ${expected[i]}`);count++;}
}
console.log(JSON.stringify({presets:650,syntheticPatches:1536,cases:count/5,filterValues:count,maxError,releaseVelocityChecks:'passed'},null,2));
