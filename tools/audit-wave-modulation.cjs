const fs=require('fs'),r=require('./reference.cjs');const presets=require('../presets/factory-decoded.json');const examples=[];
for(const x of presets){const p=x.patch,routes=[];for(const[s,...pairs]of r.matrixSlots)for(const[d,a]of pairs)if(p[s]&&[8,13].includes(p[d])&&p[a]!==64)routes.push({source:p[s],destination:p[d],amount:p[a]});for(const[s,d,a]of r.matrixLfos)if([8,13].includes(p[d])&&p[a]!==64)routes.push({source:s,destination:p[d],amount:p[a]});if(!routes.length)continue;
 const waves=[new Set(),new Set()];let simultaneousMax=0;
 // Sweep one source at a time; this is a sampled lower bound, not all combinations.
 for(const source of new Set(routes.map(x=>x.source)))for(let step=0;step<=128;step++){
 const distinct=new Set();for(const velocity of [20,50,80,110]){const sources=Array(28).fill(0);sources[24]=velocity/127;sources[source]=source===24?velocity/127:(step-64)/64;const effective={...p,_osc1Pitch:0,_osc2Pitch:0,_channelVolume:0};new r.Matrix().apply(p,sources,effective,[]);for(let i=0;i<2;i++){const wave=Math.min(63,Math.max(0,Math.round(effective[i?'osc2Wave':'osc1Wave'])));waves[i].add(wave);distinct.add(wave);}if(p.osc3Mode>=4)distinct.add(p.osc3Mode-4);}simultaneousMax=Math.max(simultaneousMax,distinct.size);
 }
 examples.push({bank:String.fromCharCode(64+x.bank),program:x.program,name:x.name,routes,sampledWaves: waves.map(x=>[...x].sort((a,b)=>a-b)),sampledSimultaneousDistinctWaves:simultaneousMax});
}
console.log(JSON.stringify({presetsWithWaveRoutes:examples.length,method:'One route source swept -1..1, four velocities, all oscillator wave choices counted even when shape can mask them; sampled lower bounds only.',fullWaveBytes:14*2049*4,threeCachesBytes:3*14*2049*4,all64WavesBytes:64*14*2049*4,freeNormalRam:7464,freePool:16228,additionalFullCacheShortfall:14*2049*4-7464-16228,examples},null,2));
