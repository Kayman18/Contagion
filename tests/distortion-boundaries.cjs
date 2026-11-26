const fs=require('fs'),cp=require('child_process'),assert=require('assert');
(async()=>{
 const ref=await require('../fx-research/distortion-reference.cjs')(process.argv[2]);
 const lines=[],expected=[],steps=[];const bits=new DataView(new ArrayBuffer(4));
 for(let level=0;level<128;level++){
  const at=level/16,i=Math.floor(at),f=at-i,d=ref.d;
  const q=(1/4096)/(d.getFloat64(17284152+i*8,true)+f*(d.getFloat64(17284152+(i+1)*8,true)-d.getFloat64(17284152+i*8,true)));
  ref.reset(8,level);lines.push(`8 ${level} 2 0`);
  // Every quantization boundary in [-8,8], with two float32 neighbors each side.
  for(let n=0;(n+.5)*q<=8;n++){
   bits.setFloat32(0,(n+.5)*q,true);const u=bits.getUint32(0,true);
   for(let delta=-2;delta<=2;delta++)for(const sign of [-1,1]){
    bits.setUint32(0,u+delta,true);const x=sign*bits.getFloat32(0,true);
    lines.push(`8 ${level} 0 ${x}`);expected.push(ref.sample(x,0));steps.push(q*.37164306640625);
   }
  }
 }
 fs.writeFileSync('build/distortion-boundary-input.txt',lines.join('\n')+'\n');
 const fd=fs.openSync('build/distortion-boundary-input.txt','r');const data=cp.execFileSync('build/distortion',{stdio:[fd,'pipe','inherit'],maxBuffer:40000000});fs.closeSync(fd);
 assert.equal(data.length,expected.length*4);let max=0,stepErrors=0;
 for(let i=0;i<expected.length;i++){const y=data.readFloatLE(i*4),e=Math.abs(y-expected[i]);assert(Number.isFinite(y));max=Math.max(max,e);if(e>steps[i]*.25)stepErrors++;}
 assert.equal(stepErrors,0);assert(max<0.000003);
 console.log(JSON.stringify({intensities:128,input_range:[-8,8],neighbors_per_boundary:5,both_signs:true,samples:expected.length,wrong_quantization_steps:stepErrors,max_error:max,pass:true},null,2));
})();
