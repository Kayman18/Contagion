const fs=require('fs'),path=require('path'), r=require('./reference.cjs');
const fmt=v=>{let s=Math.fround(v).toPrecision(9);return s+'f'};
let out='/* Generated from reference/processor.js by tools/generate_tables.cjs. */\n';
for(const key of ['cutoff','damping','damping4','filterGain','bpGain','attack','decay','release','sustain','filterAttack','detune','envAmount','linkedCutoff','sourceGain','oscVolume','patchVolume','punchLevel','portamento','glideCoef','osc2EnvCents','lfoRate','lfoPitch','lfoPW','lfoResonance','lfoFilterGain','lfo3Pitch','lfo3Fade','lfo3Rate','detune3'])
 out+=`static const float vc_${key}[129]={\n`+Array.from({length:129},(_,i)=>fmt(r[key](Math.min(key==='cutoff'?128:127,i)))).join(',')+'\n};\n';
for(const key of ['gain','hz'])out+=`static const float vc_noise_${key}[128]={`+Array.from({length:128},(_,i)=>fmt(r.noiseColor(i)[key])).join(',')+'};\n';
out+='static const float vc_slope_rate[128]={'+Array.from({length:128},(_,i)=>fmt(r.envSlope(i)[1])).join(',')+'};\n';
out+='static const float vc_punchWave[12]={'+r.punchWave.map(fmt).join(',')+'};\n';
out+='#ifndef VC_DYNAMIC_WAVES\nstatic const float vc_waves[4][14][2049]={\n';
for(let w=0;w<4;w++)out+='{\n'+r.tables(w).map(t=>'{'+Array.from(t,fmt).join(',')+'}').join(',\n')+'},\n';
out+='};\nstatic const float *const vc_wave_views[4][14]={\n'+Array.from({length:4},(_,w)=>'{'+Array.from({length:14},(_,b)=>`vc_waves[${w}][${b}]`).join(',')+'}').join(',\n')+'};\n#endif\n';fs.writeFileSync(path.join(__dirname,'../engine/tables.inc'),out);
