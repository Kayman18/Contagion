const fs=require('fs'),path=require('path'),r=require('./reference.cjs');
const f=x=>Math.fround(x).toPrecision(9)+'f';
let s='/* Captured source Ge tables and coefficient curves. Generated; do not edit. */\n';
s+='static const float levels[5]={'+r.satLevels.map(f).join(',')+'};\n';
s+='static const float curves[8][5][17]={'+r.satTables.map(c=>'{'+c.map(row=>'{'+row.map(f).join(',')+'}').join(',')+'}').join(',')+'};\n';
for(const [name,fn] of [['rates',r.satRate],['lows',r.satLow],['highs',r.satHigh]])s+=`static const float ${name}[128]={`+Array.from({length:128},(_,i)=>f(fn(Math.max(64,i)))).join(',')+'};\n';
fs.writeFileSync(path.join(__dirname,'../engine/saturation.inc'),s);
