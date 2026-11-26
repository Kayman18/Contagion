const fs=require('fs'),path=require('path');
(async()=>{const {x,d}=await require('./distortion-reference.cjs')(process.argv[2]);
 const q=addr=>(d.getInt32(addr,true)<<8>>8)/8388608;
 const tab=(addr,n)=>Array.from({length:n},(_,i)=>q(addr+i*4));
 const fmt=n=>{let s=Math.fround(n).toPrecision(9);return s+'f'};
 const array=(name,a,type='float')=>`static const ${type} ${name}[${a.length}]={\n${a.map(v=>type==='float'?fmt(v):v).join(',')}\n};\n`;
 let out='/* Extracted from original FX WASM; see distortion-reference.cjs hash guard. */\n';
 out+=array('dist_drive',tab(4320974*4,9));out+=array('dist_gain',tab(17283932,36));out+=array('dist_curves',tab(17284224,4*513));
 out+=array('dist_gain5',tab(4321019*4,9));out+=array('dist_gain6',tab(4321028*4,9));
 out+=array('dist_curve6',Array.from({length:129},(_,i)=>d.getInt16(17240324+i*2,true)),'int16_t');
 out+=array('dist_step',Array.from({length:128},(_,i)=>d.getUint32(4323108*4+i*4,true)),'uint32_t');
 const quantum=[],coeff=[];
 for(let n=0;n<128;n++){const at=n/16,i=Math.floor(at),f=at-i;quantum.push(1/4096/(d.getFloat64(2160519*8+i*8,true)+f*(d.getFloat64(2160519*8+(i+1)*8,true)-d.getFloat64(2160519*8+i*8,true))));}
 const bits=new DataView(new ArrayBuffer(8)),qbits=[];for(const q of quantum){bits.setFloat64(0,q,true);qbits.push(bits.getUint32(0,true),bits.getUint32(4,true));}
 out+=array('dist_quantum_bits',qbits,'uint32_t');
 out+=array('dist_quantum',quantum);out+=array('dist_quantum_inverse',quantum.map(q=>1/q));
 for(let i=0;i<32;i++){x.coeff(1024,i);coeff.push(d.getFloat64(1024,true),d.getFloat64(1032,true));}out+=array('dist_filter_coeff',coeff);
 fs.writeFileSync(path.resolve(__dirname,'../engine/distortion.inc'),out);
 console.log(JSON.stringify({float_table_bytes:(9+36+2052+9+9+128+128+64)*4,integer_table_bytes:129*2+128*4+256*4,total_table_bytes:((9+36+2052+9+9+128+128+64)*4+129*2+128*4+256*4)}));
})();
