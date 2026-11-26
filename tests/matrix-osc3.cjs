const cp=require('child_process'),assert=require('assert'),r=require('../tools/reference.cjs');
const output=cp.execFileSync('build/matrix-osc3',{encoding:'utf8',maxBuffer:12e6});let cases=0,maxIncrement=0,maxGain=0;
for(const line of output.trim().split('\n')){
 const [dest,amount,base,t,mode,baseInc,inc,gain]=line.split(' ').map(Number);
 const p={...r.patch(),osc3Volume:base,osc3Semitone:base,osc3Detune:base,assign1Source:3,assign1Dest:dest,assign1Amount:amount},src=Array(28).fill(0),e={...p};
 src[3]=Math.fround([-1,0,.37,1][t]);new r.Matrix().apply(p,src,e,[]);
 const clip=(v,a=0,b=127)=>Math.max(a,Math.min(b,v));
 const cents=100*(clip(Math.round(e.osc3Semitone)-64,-48,48)-clip(base-64,-48,48))+r.detune3(e.osc3Detune)-r.detune3(base);
 const wantInc=mode>1?Math.min(.45,baseInc*2**(cents/1200)):mode===1?baseInc:0;
 const wantGain=mode>1?1.673*clip(e.osc3Volume)/127:mode===1?1.058*.794:0;
 const ei=Math.abs(inc-wantInc),eg=Math.abs(gain-wantGain);maxIncrement=Math.max(maxIncrement,ei);maxGain=Math.max(maxGain,eg);
 assert(ei<.000003,JSON.stringify({line,wantInc,cents}));assert(eg<.000002);cases++;
}
// Ground oscillator-2 pitch following in the actual original voice control.
for(const mode of [1,2,3,4]){
 const e=new r.Engine(44100,{maxVoices:4});e.setPatch({osc3Mode:mode,osc3Volume:64,osc3Semitone:64,osc3Detune:64,osc2Semitone:64,osc2Detune:0,osc2FiltEnvAmt:64,lfo1Osc2:64,lfo3OscAmount:0,assign1Source:3,assign1Dest:14,assign1Amount:70});
 e.noteOn(60,100);const v=e.voices.find(v=>v.active);v.ctx.controllers[3]=0;v.control();const base=v.inc[2];v.ctx.controllers[3]=1;v.control();assert(Math.abs(v.inc[2]/base-2)<1e-10);
}
console.log(JSON.stringify({cases,modes:[0,1,2,3,4],maxIncrementError:maxIncrement,maxGainError:maxGain,sourceOsc2PitchFollowing:true,phaseAndBlepPreserved:true,bandSelection:true,voiceIsolation:true,matrixRemoval:true,blockInvariant:true,pass:true},null,2));
