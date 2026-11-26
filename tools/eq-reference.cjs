const fs=require('fs'),path=require('path'),assert=require('assert'),crypto=require('crypto');
module.exports=async function(wabtPath){
 const root=path.resolve(__dirname,'..'),raw=fs.readFileSync(root+'/reference/virus_fx.wasm');
 assert.equal(crypto.createHash('sha256').update(raw).digest('hex'),'2fd986aab9caad2eee8186d11cfd272eb99180e4cb70c51a4ad47d49dc5d0e17');
 const wabt=await require(wabtPath)(),m=wabt.readWasm(raw,{});let wat=m.toText({foldExprs:false,inlineExport:false});
 wat=wat.slice(0,wat.lastIndexOf(')'))+'(export "probe" (func 80))\n(export "eqprobe" (func 81))\n(export "coeff" (func 91))\n)';
 const p=wabt.parseWat('distortion-probe',wat);p.resolveNames();p.validate();let x;
 const wasi={environ_sizes_get(a,b){const d=new DataView(x.memory.buffer);d.setUint32(a,0,true);d.setUint32(b,0,true);return 0},environ_get:()=>0,fd_close:()=>0,fd_seek:()=>0,fd_write:()=>0,proc_exit:c=>{throw Error(c)}};
 x=new WebAssembly.Instance(new WebAssembly.Module(p.toBinary({}).buffer),{wasi_snapshot_preview1:wasi}).exports;x._initialize();
 const d=new DataView(x.memory.buffer),address=17596312;
 return {x,d,reset(mode,level){new Uint8Array(x.memory.buffer,address,80).fill(0);d.setInt32(address,mode,true);d.setInt32(address+4,level,true)},set(mode,level){d.setInt32(address,mode,true);d.setInt32(address+4,level,true)},sample(v,ch){return x.probe(address,v,ch)}};
};
