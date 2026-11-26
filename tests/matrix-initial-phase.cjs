// Destination 88 is accepted by Matrix, but the captured Voice reads base phase.
const assert=require('assert'),r=require('../tools/reference.cjs');let cases=0,changed=0;
for(let base=1;base<=127;base++)for(const amount of [0,32,64,96,127])for(const source of [-1,0,.37,1]){
 const e=new r.Engine(44100,{maxVoices:4});e.setPatch({oscInitPhase:base,osc3Mode:1,assign1Source:3,assign1Dest:88,assign1Amount:amount});e.ctx.controllers[3]=source;e.noteOn(60,100);const v=e.voices.find(v=>v.active);assert(v);const expected=[0,base/128,1-base/128];assert.deepEqual(v.oscillators.map(o=>o.phase),expected);if(v.eff.oscInitPhase!==base)changed++;v.control();assert.deepEqual(v.oscillators.map(o=>o.phase),expected);cases++;
}
assert(changed>0);console.log(JSON.stringify({cases,effectivePhaseChangedCases:changed,noteStartUsesBasePhase:true,controlDoesNotApplyEffectivePhase:true,pass:true},null,2));
