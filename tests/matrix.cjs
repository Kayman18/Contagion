const fs=require('fs'),path=require('path'),cp=require('child_process'),assert=require('assert');
const root=path.join(__dirname,'..'),ref=require('../tools/reference.cjs');
const presets=JSON.parse(fs.readFileSync(path.join(root,'presets/factory-decoded.json')));
const pairs=ref.matrixSlots.flatMap(([source,...pairs])=>pairs.map(([dest,amount])=>({source,dest,amount})))
 .concat(ref.matrixLfos.map(([source,dest,amount])=>({source,dest,amount})));
const bin=path.join(root,'build/matrix-test'),chunks=[],expected=[];
let seed=0x15349;
function rand(){seed=(Math.imul(seed,1664525)+1013904223)>>>0;return seed/4294967296;}
function routeBytes(p){return Buffer.from(pairs.flatMap(r=>[typeof r.source==='number'?r.source:p[r.source],p[r.dest],p[r.amount]]));}
function run(args,input){let r=cp.spawnSync(bin,args,{input,maxBuffer:40*1024*1024});
 if(r.status!==0)throw Error(`native runner ${r.status}: ${r.stderr}`);return r.stdout;}
assert.deepStrictEqual(run(['factory']),Buffer.concat(presets.map(p=>routeBytes(p.patch))));
assert.deepStrictEqual(run(['cc']).toString().trim().split('\n').map(Number),Array.from({length:128},(_,cc)=>ref.controllerSources[cc]??-1));
function add(p,sources){
 const base=ref.matrixDestinations.map(([,key])=>Math.fround(key?(p[key]??0):0));
 const buf=Buffer.alloc(33+28*4+123*4);routeBytes(p).copy(buf);
 sources=sources.map(Math.fround);sources.forEach((v,i)=>buf.writeFloatLE(v,33+i*4));
 base.forEach((v,i)=>buf.writeFloatLE(v,33+112+i*4));chunks.push(buf);
 const out={...p,_osc1Pitch:0,_osc2Pitch:0,_channelVolume:0},touched=[];
 new ref.Matrix().apply(p,sources,out,touched);
 expected.push(ref.matrixDestinations.map(([,key])=>key?(out[key]??0):0));
 // Reusing effective values must reset previous destinations in original.
 const matrix=new ref.Matrix();matrix.apply(p,sources,out,touched);
 for(let i=0;i<123;i++){let key=ref.matrixDestinations[i][1];if(key)assert.equal(out[key]??0,expected.at(-1)[i]);}
}
for(const {patch} of presets){
 for(let source=0;source<28;source++)for(const level of [-1,0,1,2]){
  const values=Array(28).fill(0);values[source]=level;add(patch,values);
 }
 add(patch,Array.from({length:28},()=>rand()*2-1));
}
// Exercise arbitrary chains, duplicate destinations and amount saturation.
for(let n=0;n<5000;n++){
 const p=ref.patch();
 for(const r of pairs){if(typeof r.source==='string')p[r.source]=Math.floor(rand()*28);
  p[r.dest]=Math.floor(rand()*123);p[r.amount]=Math.floor(rand()*128);}
 add(p,Array.from({length:28},()=>rand()*3-1));
}
const output=run(process.argv.includes("--rounded")?["rounded"]:process.argv.includes("--filters")?["filters"]:[],Buffer.concat(chunks));assert.equal(output.length,expected.length*123*4);
let maxError=0;
for(let row=0;row<expected.length;row++)for(let col=0;col<123;col++){
 const actual=output.readFloatLE((row*123+col)*4),want=process.argv.includes("--rounded")&&(col===36||col===41)?Math.round(Math.max(0,Math.min(127,expected[row][col]))):expected[row][col],error=Math.abs(actual-want);
 maxError=Math.max(maxError,error);
 if(!Number.isFinite(actual)||error>0.00025)throw Error(`row ${row}, destination ${col}: ${actual} != ${want}`);
}
console.log(JSON.stringify({cases:expected.length,comparedValues:expected.length*123,maxError,
 factoryRoutesVerified:650*11,controllerMappingsVerified:128,arpPresets:0},null,2));
