// Exact source coefficients and sine constants; no waveform quantisation.
const fs=require('fs'),path=require('path'),r=require('./reference.cjs');
const dir=path.join(__dirname,'../data');fs.mkdirSync(dir,{recursive:true});
const coefficients=Buffer.alloc(64*256*8);let at=0;
for(const wave of r.coefficients)for(const part of wave)for(const v of part){coefficients.writeDoubleLE(v,at);at+=8;}
fs.writeFileSync(dir+'/wave-coefficients.f64',coefficients);
fs.writeFileSync(dir+'/wave-sines.f64',Buffer.from(r.sines.buffer));
fs.writeFileSync(dir+'/source-waves.f32',Buffer.concat(Array.from({length:64},(_,w)=>r.tables(w).map(t=>Buffer.from(t.buffer))).flat()));
