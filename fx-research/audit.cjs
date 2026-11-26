// Run the captured original FX, inspect its own buffer descriptors, and test
// float32 storage round trips. No firmware or reference asset is modified.
const fs=require('fs'),crypto=require('crypto'),path=require('path');
const root=path.resolve(__dirname,'..'),bytes=fs.readFileSync(root+'/reference/virus_fx.wasm');
const hash=crypto.createHash('sha256').update(bytes).digest('hex');
if(hash!=='2fd986aab9caad2eee8186d11cfd272eb99180e4cb70c51a4ad47d49dc5d0e17')throw Error('Reference changed: offsets require revalidation');
const r=require(root+'/tools/reference.cjs'),js=fs.readFileSync(root+'/reference/processor.js','utf8');
const keys=js.match(/wt=\/\* @__PURE__ \*\/ `([^`]+)`\.split/)[1].split('.');
function create(){let x;const wasi={environ_sizes_get(a,b){let d=new DataView(x.memory.buffer);d.setUint32(a,0,true);d.setUint32(b,0,true);return 0},environ_get:()=>0,fd_close:()=>0,fd_seek:()=>0,fd_write:()=>0,proc_exit:c=>{throw Error('WASI exit '+c)}};x=new WebAssembly.Instance(new WebAssembly.Module(bytes),{wasi_snapshot_preview1:wasi}).exports;const initial=x.memory.buffer.byteLength;x._initialize();return {x,initial};}
const {x,initial}=create();let d=new DataView(x.memory.buffer);
// Identified from original constructor func94, reset func72, processors 84–86.
const vectors=[['chorus L',17297344],['chorus R',17297364],['delay A',17297560],['delay B',17297580]].map(([name,at])=>{const start=d.getUint32(at,true),end=d.getUint32(at+4,true),capacity=d.getUint32(at+8,true);if(end!==capacity||(end-start)%8)throw Error(name);return {name,start,bytes:end-start,samples:(end-start)/8};});
const regions=[...vectors,{name:'reverb main',start:17297840,bytes:262144,samples:32768},{name:'reverb diffusion',start:17559984,bytes:3072,samples:384}];
if(vectors.map(v=>v.bytes).join()!=='8192,8192,262144,262144')throw Error('Unexpected vector allocation');
const patch=r.patch();function set(p){keys.forEach((k,i)=>x.fx_set(i,p[k]));x.fx_set_tempo(120);x.fx_process(0);x.fx_reset();}
const precision=[];let seed=12345;
for(const mode of [0,1,2,3,4,5,10,25]){
 set({...patch,analogBoostIntensity:0,chorusMix:100,chorusFeedback:100,delayReverbMode:mode,effectSend:100,delayFeedback:100});
 const stats=regions.map(v=>({...v,nonzero:0,inexact_f32:0,max_abs_error:0}));
 const l=new Float32Array(x.memory.buffer,x.fx_left(),256),rr=new Float32Array(x.memory.buffer,x.fx_right(),256);
 let finite=true,energy=0;
 for(let block=0;block<1024;block++){
  for(let j=0;j<256;j++){seed^=seed<<13;seed^=seed>>>17;seed^=seed<<5;let q=block<128?((seed>>>0)/4294967296-.5)*.2:0;l[j]=q;rr[j]=-q*.5;}
  x.fx_process(256);for(let j=0;j<256;j++){finite&&=Number.isFinite(l[j])&&Number.isFinite(rr[j]);energy+=l[j]*l[j]+rr[j]*rr[j];}
  if(block%64===63)for(const s of stats){const a=new Float64Array(x.memory.buffer,s.start,s.samples);for(const v of a){if(!Number.isFinite(v))throw Error('nonfinite buffer');s.nonzero+=v!==0;const error=Math.abs(v-Math.fround(v));s.inexact_f32+=error!==0;s.max_abs_error=Math.max(s.max_abs_error,error);}}
 }
 if(!finite||energy<=0)throw Error('invalid render');precision.push({mode,frames:262144,finite,energy,buffers:stats.map(({name,nonzero,inexact_f32,max_abs_error})=>({name,nonzero,inexact_f32,max_abs_error}))});
}
const normalFree=98304-84060,poolFree=344064-327836;
const original={chorus:16384,delay:524288,reverb:265216};
const float32={chorus:8192,delay:262144,reverb:132608};
console.log(JSON.stringify({reference_sha256:hash,fx_parameter_order:keys,wasm_initial_bytes:initial,wasm_initialized_bytes:x.memory.buffer.byteLength,wasm_reserved_stack_bytes:16777216,regions,original_buffer_bytes:original,float32_buffer_bytes:float32,current_target:{normal_free:normalFree,pool_free:poolFree,combined_free:normalFree+poolFree,oscillator_cache_bytes:344232,application_flash_free:581564-577600},shared_delay_reverb:true,float32_chorus_plus_shared_fx:270336,minimum_additional_bytes_needed:270336-normalFree-poolFree,includes_fx_state_and_alignment:false,precision,pass:true},null,2));
