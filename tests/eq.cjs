const fs=require('fs'),cp=require('child_process'),assert=require('assert');
(async()=>{const {x,d}=await require('../fx-research/eq-reference.cjs')(process.argv[2]),base=17296936;
 const lines=[],expected=[],types=[],cases=[];let seed=12345;
 const neutral=[64,64,64,64,64,64,64];
 for(let k=0;k<7;k++)for(let n=0;n<128;n++){let p=[...neutral];p[k]=n;cases.push(p);}
 for(let k=0;k<128;k++)cases.push(Array.from({length:7},(_,j)=>(k>>j&1)?127:0));
 for(let k=0;k<256;k++)cases.push(Array.from({length:7},()=>{seed^=seed<<13;seed^=seed>>>17;seed^=seed<<5;return seed>>>25}));
 const offsets=[96,104,112,120,128,240,248,256,328,336,344,400];
 const extended=process.env.EQ_STRESS==='1',live=process.env.EQ_LIVE==='1';let first=true;
 if(extended){cases.length=0;for(const f of [0,1,32,127])for(const g of [0,64,127])cases.push([127,0,g,f,127,127,127]);}
 for(const p of cases){
  if(!live||first)x.fx_reset();x.fx_set(2,0);x.fx_set(3,64);p.forEach((v,i)=>x.fx_set(i+4,v));x.fx_process(0);
  // Clear every filter history while retaining configured coefficients.
  if(!live||first)for(const [off,n] of [[48,4],[160,8],[280,4],[368,4]])for(let i=0;i<n;i++)d.setFloat64(base+off+i*8,0,true);
  lines.push((!live||first?'Q ':'U ')+p.join(' '));first=false;for(let i=0;i<12;i++){expected.push(d.getFloat64(base+offsets[i],true));types.push(i<5?'mid':'coef');}
  for(let k=0;k<(extended?131072:512);k++)for(let ch=0;ch<2;ch++){
   seed^=seed<<13;seed^=seed>>>17;seed^=seed<<5;
   const input=Math.fround(k===0?.125:k<128?Math.sin(k*.113+ch)*.1:k<256?((seed>>>0)/4294967296-.5)*.2:0);
   lines.push(`S ${ch} ${input}`);expected.push(x.eqprobe(base,input,ch));types.push('audio');
  }
 }
 fs.writeFileSync('build/eq-input.txt',lines.join('\n')+'\n');const fd=fs.openSync('build/eq-input.txt','r');const data=cp.execFileSync('build/eq',{stdio:[fd,'pipe','inherit'],maxBuffer:40000000});fs.closeSync(fd);assert.equal(data.length,expected.length*4);
 const stats={mid:{max:0,changed:0},coef:{max:0},audio:{max:0,rms:0,count:0}};
 for(let i=0;i<expected.length;i++){let y=data.readFloatLE(i*4),e=Math.abs(y-expected[i]),s=stats[types[i]];assert(Number.isFinite(y));if(e>s.max){s.max=e;s.worst={index:i,case:Math.floor(i/1036),parameters:cases[Math.floor(i/1036)],expected:expected[i],actual:y};}if(types[i]==='mid')s.changed+=y!==expected[i];if(types[i]==='audio'){s.rms+=e*e;s.count++;}}
 stats.audio.rms=Math.sqrt(stats.audio.rms/stats.audio.count);assert.equal(stats.mid.changed,0);assert(stats.coef.max<0.000005);console.log(JSON.stringify({cases:cases.length,stress:extended,live_changes:live,pass:stats.audio.max<0.00001,limits:{coefficient:0.000005,audio_absolute:0.00001},stats},null,2));assert(stats.audio.max<0.00001);
})();
