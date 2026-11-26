const fs=require('fs'),cp=require('child_process'),assert=require('assert');
(async()=>{
 const w=await require(process.argv[2])(),raw=fs.readFileSync('reference/virus_fx.wasm');
 let wat=w.readWasm(raw,{}).toText({foldExprs:false,inlineExport:false});
 const start=wat.indexOf('  (func (;81;)'),end=wat.indexOf('    f64.store offset=56',start)+'    f64.store offset=56'.length;
 let probe=wat.slice(start,end).replace('(func (;81;)','(func $boostprobe')+'\n local.get 1)\n(export "boostprobe" (func $boostprobe))\n';
 wat=wat.slice(0,wat.lastIndexOf(')'))+probe+')';
 const m=w.parseWat('boost',wat);m.resolveNames();m.validate();let x;
 const wasi={environ_sizes_get(a,b){let d=new DataView(x.memory.buffer);d.setUint32(a,0,true);d.setUint32(b,0,true);return 0},environ_get:()=>0,fd_close:()=>0,fd_seek:()=>0,fd_write:()=>0,proc_exit:c=>{throw Error(c)}};
 x=new WebAssembly.Instance(new WebAssembly.Module(m.toBinary({}).buffer),{wasi_snapshot_preview1:wasi}).exports;x._initialize();
 const d=new DataView(x.memory.buffer),base=17296936,cases=[];
 for(let t=0;t<128;t++)for(let a=0;a<128;a++)cases.push([a,t,64,1]);
 for(let t of [0,1,64,127])for(let a of [0,1,64,127])cases.push([a,t,65536,1]);
 for(let j=0;j<256;j++)cases.push([j%128,(j*37)%128,256,0]);
 const file='build/boost-input.txt';fs.writeFileSync(file,cases.map(a=>a.join(' ')).join('\n'));
 const fd=fs.openSync(file,'r'),native=cp.execFileSync('build/boost',{stdio:[fd,'pipe','inherit'],maxBuffer:64000000});fs.closeSync(fd);
 let at=0,seed=12345,max=0,sum=0,n=0,coefmax=0;
 for(const [a,t,frames,reset] of cases){
  x.fx_set(2,a);x.fx_set(3,t);x.fx_process(0);
  if(reset)for(let j=0;j<4;j++)d.setFloat64(base+48+j*8,0,true);
  for(let j=0;j<3;j++){let e=d.getFloat64(base+8+j*8,true),v=native.readFloatLE(at);at+=4;coefmax=Math.max(coefmax,Math.abs(v-e));assert.equal(v,e);}
  for(let i=0;i<frames;i++)for(let ch=0;ch<2;ch++){
   seed^=seed<<13;seed^=seed>>>17;seed^=seed<<5;
   const input=i===0?.125:i<128?((seed&65535)-32768)/65536:0;
   const expected=x.boostprobe(base,input,ch),actual=native.readFloatLE(at);at+=4;
   assert(Number.isFinite(actual));let e=Math.abs(actual-expected);max=Math.max(max,e);sum+=e*e;n++;
  }
 }
 assert.equal(at,native.length);
 console.log(JSON.stringify({cases:cases.length,coefficientMaxError:coefmax,samples:n,maxError:max,rmsError:Math.sqrt(sum/n),limit:.00001,pass:max<.00001},null,2));
 assert(max<.00001);
})();
