// Extract and losslessly encode the original Q23 table. No audio-time decoding.
const fs=require('fs'),assert=require('assert');
(async()=>{
 const {d}=await require('./eq-reference.cjs')(process.argv[2]);
 const data=[],offsets=[],feedback=[];
 const value=(t,a,k)=>d.getInt32(16779600+((t*128+a)*3+k)*4,true)<<8>>8;
 for(let t=0;t<128;t++){
  offsets.push(data.length);feedback.push(value(t,1,2));
  assert.equal(value(t,0,0),4194304);assert.equal(value(t,0,1),0);assert.equal(value(t,0,2),0);
  const previous=[0,0],delta=[0,0];
  for(let a=1;a<128;a++){
   assert.equal(value(t,a,2),feedback[t]);
   for(let k=0;k<2;k++){
    const x=value(t,a,k),next=x-previous[k],diff=next-delta[k];previous[k]=x;delta[k]=next;
    let n=diff>=0?2*diff:-2*diff-1;
    while(n>=128){data.push((n&127)|128);n>>>=7;}data.push(n);
   }
  }
 }
 assert(data.length<65536);
 fs.writeFileSync('engine/boost.inc','/* Lossless second differences of original Q23 Analog Boost table. */\nstatic const uint16_t boost_offsets[128]={'+offsets.join(',')+'};\nstatic const int32_t boost_feedback[128]={'+feedback.join(',')+'};\nstatic const uint8_t boost_data[]={'+data.join(',')+'};\n');
 console.log(JSON.stringify({combinations:16384,encodedBytes:data.length+256+512,lossless:true}));
})();
