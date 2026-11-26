// Test numerical equivalence of float32 delay/reverb STORAGE, while preserving
// original double arithmetic. This is an instrumented reference, not a native port.
const fs=require('fs'),path=require('path'),assert=require('assert');
(async()=>{
 const wabt=await require(process.argv[2])(),root=path.resolve(__dirname,'..');
 const raw=fs.readFileSync(root+'/reference/virus_fx.wasm'),audit=JSON.parse(fs.readFileSync(__dirname+'/audit.json'));
 const module=wabt.readWasm(raw,{readDebugNames:true});module.generateNames();module.applyNames();
 let wat=module.toText({foldExprs:false,inlineExport:false});
 const ranges=audit.regions.filter(r=>!r.name.startsWith('chorus'));
 // Route scalar double stores through a range-checked store. Outside the four
 // delay/reverb regions, including all chorus/state writes, keep full precision.
 wat=wat.replace(/f64\.store(?: offset=(\d+))?(?: align=\d+)?/g,(_,offset)=>`i32.const ${offset||0}\n call $storage_probe`);
 const cond=r=>`local.get $address\ni32.const ${r.start}\ni32.ge_u\nlocal.get $address\ni32.const ${r.start+r.bytes}\ni32.lt_u\ni32.and`;
 const condition=ranges.map(cond).reduce((s,c,i)=>s+c+(i?'\ni32.or':'')+'\n','');
 const fn=`(func $storage_probe (param $address i32) (param $value f64) (param $offset i32)
 local.get $address local.get $offset i32.add local.set $address
 ${condition}
 if local.get $value f32.demote_f64 f64.promote_f32 local.set $value end
 local.get $address local.get $value f64.store)\n`;
 wat=wat.slice(0,wat.lastIndexOf(')'))+fn+')';
 const instrumented=wabt.parseWat('storage-probe.wat',wat);instrumented.resolveNames();instrumented.validate();const binary=instrumented.toBinary({}).buffer;
 function instantiate(bytes){let x;const wasi={environ_sizes_get(a,b){const d=new DataView(x.memory.buffer);d.setUint32(a,0,true);d.setUint32(b,0,true);return 0},environ_get:()=>0,fd_close:()=>0,fd_seek:()=>0,fd_write:()=>0,proc_exit:c=>{throw Error(c)}};x=new WebAssembly.Instance(new WebAssembly.Module(bytes),{wasi_snapshot_preview1:wasi}).exports;x._initialize();return x;}
 const ref=require(root+'/tools/reference.cjs'),cases=[];
 for(const mode of [1,2,3,4,5,10,25]){
  const a=instantiate(raw),b=instantiate(binary),p={...ref.patch(),analogBoostIntensity:0,chorusMix:100,chorusFeedback:100,delayReverbMode:mode,effectSend:100,delayFeedback:120};
  for(const x of [a,b]){audit.fx_parameter_order.forEach((k,i)=>x.fx_set(i,p[k]));x.fx_set_tempo(120);x.fx_process(0);x.fx_reset();}
  const al=new Float32Array(a.memory.buffer,a.fx_left(),256),ar=new Float32Array(a.memory.buffer,a.fx_right(),256),bl=new Float32Array(b.memory.buffer,b.fx_left(),256),br=new Float32Array(b.memory.buffer,b.fx_right(),256);
  let seed=12345,maxError=0,changed=0;
  for(let block=0;block<1024;block++){
   if(block===300||block===600)for(const x of [a,b])x.fx_set(26,block===300?110:20); // delay time transitions
   for(let j=0;j<256;j++){seed^=seed<<13;seed^=seed>>>17;seed^=seed<<5;let q=block<128?((seed>>>0)/4294967296-.5)*.2:0;al[j]=bl[j]=q;ar[j]=br[j]=-q*.5;}
   a.fx_process(256);b.fx_process(256);
   for(let j=0;j<256;j++){assert(Number.isFinite(bl[j])&&Number.isFinite(br[j]));maxError=Math.max(maxError,Math.abs(al[j]-bl[j]),Math.abs(ar[j]-br[j]));changed+=(al[j]!==bl[j])+(ar[j]!==br[j]);}
  }
  assert.equal(changed,0,'storage changed output for mode '+mode);cases.push({mode,stereo_frames:262144,changed_samples:changed,max_error:maxError});
 }
 console.log(JSON.stringify({scope:'float32 storage only, original double arithmetic retained; not a native CPU test',chorus_storage_unchanged:true,cases,pass:true},null,2));
})();
