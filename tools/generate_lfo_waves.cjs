// Exact source float32 LFO tables; firmware embeds 40 and installs 22 from its lossless oscillator bank.
const fs=require('fs'),path=require('path'),r=require('./reference.cjs');
const tables=Array.from({length:62},(_,i)=>Array.from(r.tables(i+2)[0]));
const f=x=>{let s=x.toPrecision(9);if(!/[.e]/.test(s))s+='.0';return s+'f';};
const rows=tables.map(t=>'{'+t.map(f).join(',')+'}');
fs.writeFileSync(path.join(__dirname,'../engine/lfo_waves.inc'),'/* Generated exact float32 tables. */\nstatic const float vc_lfo_waves[][2049]={\n'+rows.slice(0,40).join(',\n')+',\n#ifndef VC_DYNAMIC_WAVES\n'+rows.slice(40).join(',\n')+'\n#endif\n};\n');
function crc(b){let c=0xffffffff;for(let x of b){c^=x;for(let j=0;j<8;j++)c=(c>>>1)^((c&1)?0xedb88320:0);}return (~c)>>>0;}
const checks=tables.slice(40).map(t=>crc(Buffer.from(new Float32Array(t).buffer)));
fs.writeFileSync(path.join(__dirname,'../engine/lfo_extra_crc.inc'),'/* Source tables 42..63, 2049 float32 samples each. */\nstatic const uint32_t lfo_extra_crc[22]={'+checks.map(x=>'0x'+x.toString(16)+'u').join(',')+'};\n');

fs.writeFileSync(path.join(__dirname,'../data/VIROOOS-LFO-EXTRA.bin'),Buffer.concat(tables.slice(40).map(t=>Buffer.from(new Float32Array(t).buffer))));
