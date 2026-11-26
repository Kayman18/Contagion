const fs=require('fs'),cp=require('child_process'),assert=require('assert');
(async()=>{
 const ref=await require('../fx-research/distortion-reference.cjs')(process.argv[2]);
 const lines=[],expected=[],modes=[];let seed=1234567;
 function input(x){return Math.fround(x)}
 for(let mode=0;mode<12;mode++)for(let level=0;level<128;level++){
  lines.push(`${mode} ${level} 2 0`);ref.reset(mode,level);
  for(let k=0;k<512;k++)for(let ch=0;ch<2;ch++){
   seed^=seed<<13;seed^=seed>>>17;seed^=seed<<5;
   let x=input(k<129?(k-64)/32:k<256?Math.sin(k*.083+ch)*.9:k<384?((seed>>>0)/4294967296-.5)*3:0);
   lines.push(`${mode} ${level} ${ch} ${x}`);expected.push(ref.sample(x,ch));modes.push(mode);
  }
 }
 // Carry histories through parameter and mode changes, then long zero tails.
 ref.reset(0,0);lines.push('0 0 2 0');
 for(const mode of [7,10,11,9,1,6,0,7,11,10])for(const level of [0,1,63,64,126,127]){
  ref.set(mode,level);lines.push(`${mode} ${level} 3 0`);
  for(let k=0;k<4096;k++)for(let ch=0;ch<2;ch++){
   const x=input(k<256?Math.sin(k*.057+ch)*1.2:0);
   lines.push(`${mode} ${level} ${ch} ${x}`);expected.push(ref.sample(x,ch));modes.push(mode);
  }
 }
 fs.writeFileSync('build/distortion-input.txt',lines.join('\n')+'\n');
 const fd=fs.openSync('build/distortion-input.txt','r');let data=cp.execFileSync('build/distortion',{stdio:[fd,'pipe','inherit'],maxBuffer:20000000});fs.closeSync(fd);
 assert.equal(data.length,expected.length*4);const stats=Array.from({length:12},(_,mode)=>({mode,samples:0,max_error:0,rms_error:0,worst:null}));
 for(let i=0;i<expected.length;i++){const y=data.readFloatLE(i*4),x=expected[i],e=Math.abs(y-x),s=stats[modes[i]];assert(Number.isFinite(y));s.samples++;s.rms_error+=e*e;if(e>s.max_error){s.max_error=e;s.worst={index:i,expected:x,actual:y};}}
 for(const s of stats)s.rms_error=Math.sqrt(s.rms_error/s.samples);
 for(const s of stats){s.status='passed';assert(s.max_error<0.00003,JSON.stringify(s));}
 console.log(JSON.stringify({ready_for_firmware:true,pending_modes:[],scope:'native float versus isolated original double distortion DSP; all 128 intensities, stereo ramps, sine, noise and zero tails',samples:expected.length,stats},null,2));
})();
