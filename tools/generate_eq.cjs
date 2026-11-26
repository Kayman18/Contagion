const fs=require('fs'),path=require('path');
(async()=>{const {x,d}=await require('./eq-reference.cjs')(process.argv[2]);
 const q=a=>(d.getInt32(a,true)<<8)>>8,ints=(a,n)=>Array.from({length:n},(_,i)=>q(a+i*4));
 const arr=(name,a,type)=>`static const ${type} ${name}[${a.length}]={\n${a.map(v=>type==='float'?Math.fround(v).toPrecision(9)+'f':v).join(',')}\n};\n`;
 let s='/* Original FX coefficients; generated from hash-checked WASM. */\n';
 s+=arr('eq_gain_a',ints(4276820*4,17),'int32_t');s+=arr('eq_gain_b',ints(4276837*4,17),'int32_t');
 s+=arr('eq_norm',ints(4309896*4,17).map(v=>v/8388608),'float');
 const coeff=[];for(let i=0;i<32;i++){x.coeff(1024,i);coeff.push(d.getFloat64(1024,true),d.getFloat64(1032,true));}s+=arr('eq_frequency',coeff,'float');
 const bytes=[];for(let i=0;i<32768;i++){let n=d.getUint32(16976208+i*4,true);if(n>0xffffff)throw Error('non-q23');bytes.push(n&255,(n>>8)&255,n>>16);}
 s+=arr('eq_mid_packed',bytes,'uint8_t');
 fs.writeFileSync(path.resolve(__dirname,'../engine/eq.inc'),s);
})();
