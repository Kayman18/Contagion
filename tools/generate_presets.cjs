const fs=require('fs'),path=require('path'),vm=require('vm'),assert=require('assert'),crypto=require('crypto');
const root=path.resolve(__dirname,'..'),src=fs.readFileSync(root+'/reference/index.js','utf8');
const start=src.indexOf('var la=new Uint8Array'),end=src.indexOf('var ha=',start);assert(start>0&&end>start);
const ctx=vm.createContext({Uint8Array,Math,Number,Object});vm.runInContext(src.slice(start,end)+';globalThis.meta={da,fa,la,ua};',ctx);const {da,fa,la,ua}=ctx.meta;
// The default input retains the original C–H layout for legacy builds.
const model=process.argv[2];
const directory=process.argv[3];
if(process.argv.length!==4||!['A','B','C','ALL'].includes(model)||!directory)
 throw Error('Usage: node tools/generate_presets.cjs A|B|C|ALL bank-directory');
const models=model==='ALL'?['A','B','C']:[model];
const banks=models.flatMap(m=>Array.from({length:m==='A'?2:6},(_,i)=>{
 const bank=String.fromCharCode(65+i);
 const root=model==='ALL'?path.join(directory,`Virus-${m}`):directory;
 return {file:path.join(root,`Virus-${m}-Bank-${bank}.syx`),sourceBank:i+1,label:`${m}-${bank}`};
}));
const bankNames=banks.map(b=>b.label), bankCount=banks.length, firstBank=1;
const filenames=banks.map(b=>b.file);
const inputs=filenames.map(f=>fs.readFileSync(f));
const raw=Buffer.concat(inputs),patches=[];
for(const [fileIndex,input] of inputs.entries()){
 assert.equal(input.length,128*267,`Unexpected length: ${filenames[fileIndex]}`);
 for(let i=0;i<128;i++){
  const m=input.subarray(i*267,(i+1)*267);
  assert.deepEqual([...m.subarray(0,5)],[240,0,32,51,1]);
  assert.equal(m[6],16);
  assert.equal(m[7],banks[fileIndex].sourceBank);
  assert.equal(m[8],i);
  assert.equal(m[266],247);
  assert([...m.subarray(5,266)].every(x=>x<128));
  assert.equal([...m.subarray(5,265)].reduce((a,b)=>a+b,0)&127,m[265]);
  const bytes=m.subarray(9,265);
  const patch=Object.fromEntries(fa.map(n=>[n,Math.min(da[n].max,bytes[ua(da[n].page,da[n].no)])]));
  const name=Array.from(bytes.subarray(240,250),b=>b>=32&&b<127?String.fromCharCode(b):' ').join('').trimEnd();
  patches.push({bank:fileIndex+1,program:m[8],name,patch});
 }
}
const excludedArp=patches.filter(p=>p.patch.arpMode>0).map(({bank,program,name})=>({bank,program,name}));
const allPrograms=patches.length;const retained=patches.filter(p=>p.patch.arpMode===0);patches.length=0;patches.push(...retained);const count=patches.length;
fs.writeFileSync(root+'/presets/excluded-arp.json',JSON.stringify(excludedArp,null,2));
const header=fs.readFileSync(root+'/engine/virooos.h','utf8');const part=header.slice(header.indexOf('VC_SHAPE1'),header.indexOf('VC_PARAMS'));const ids=part.match(/VC_[A-Z0-9_]+/g);assert.equal(new Set(ids).size,ids.length);
const map=Object.fromEntries(JSON.parse(fs.readFileSync(root+'/presets/midi-map.json')).map(r=>[r.native,r.source]));
Object.assign(map,{
 VC_BOOST_LEVEL:'analogBoostIntensity',VC_BOOST_TUNE:'analogBoostTune',VC_UNISON_MODE:'unisonMode',VC_UNISON_DETUNE:'unisonDetune',VC_UNISON_PHASE:'unisonLfoPhase',VC_SPREAD:'unisonPanSpread',VC_WAVE1:'osc1Wave',VC_WAVE2:'osc2Wave',VC_SYNC:'osc2Sync',VC_FM_VELOCITY:'fmAmountVelocity',VC_LFO_CONTOUR:'lfo1Contour',VC_LFO_OSC1:'lfo1Osc1',VC_LFO_GLOBAL:'lfo1Mode',VC_LFO2_GLOBAL:'lfo2Mode',VC_LFO3_GLOBAL:'lfo3Mode',VC_LFO_CLOCK:'lfo1Clock',VC_LFO2_CLOCK:'lfo2Clock',VC_TEMPO:'clockTempo',VC_F_POLARITY1:'filter1EnvPolarity',VC_F_POLARITY2:'filter2EnvPolarity',VC_CUTOFF_LINK:'filter2CutoffLink',VC_F_KEYBASE:'filterKeytrackBase',VC_FM_MODE:'fmMode',VC_INIT_PHASE:'oscInitPhase',VC_OSC3_MODE:'osc3Mode',VC_OSC3_VOLUME:'osc3Volume',VC_OSC3_TUNE:'osc3Semitone',VC_OSC3_DETUNE:'osc3Detune',VC_LFO3_RATE:'lfo3Rate',VC_LFO3_SHAPE:'lfo3Shape',VC_LFO3_AMOUNT:'lfo3OscAmount',VC_LFO3_DEST:'lfo3Destination',VC_LFO3_FADE:'lfo3FadeIn',VC_LFO3_KEYTRACK:'lfo3Keyfollow',VC_LFO3_CLOCK:'lfo3Clock',VC_PUNCH:'punchIntensity',VC_VEL_ENV1:'filter1EnvAmtVelocity',VC_VEL_ENV2:'filter2EnvAmtVelocity',VC_VEL_RES1:'resonance1Velocity',VC_VEL_RES2:'resonance2Velocity',VC_VEL_SHAPE1:'osc1ShapeVelocity',VC_VEL_SHAPE2:'osc2ShapeVelocity',VC_VEL_PW:'pulseWidthVelocity',VC_VEL_PAN:'panVelocity',VC_AMP_VELOCITY:'ampVelocity',VC_BEND_UP:'bendUp',VC_BEND_DOWN:'bendDown',VC_BEND_SCALE:'bendScale',VC_KEY_MODE:'keyMode',VC_DIST_MODE:'distortionCurve',VC_DIST_LEVEL:'distortionIntensity',VC_EQ_LOW_GAIN:'lowEqGain',VC_EQ_LOW_FREQ:'lowEqFreq',VC_EQ_MID_GAIN:'midEqGain',VC_EQ_MID_FREQ:'midEqFreq',VC_EQ_MID_Q:'midEqQ',VC_EQ_HIGH_GAIN:'highEqGain',VC_EQ_HIGH_FREQ:'highEqFreq'});
const derived={VC_PATCH_VOLUME:()=>90,VC_LFO_MODE:p=>p.lfo1EnvMode?2:1,VC_LFO2_MODE:p=>p.lfo2EnvMode?2:1,VC_LFO3_ENABLE:p=>1,VC_EQ_ENABLE:p=>1};
const missing=ids.filter(id=>!map[id]&&!derived[id]);assert.deepEqual(missing,[]);
const used=new Set([...Object.values(map),'lfo1EnvMode','lfo2EnvMode']);const excluded=n=>/^(chorus|phaser|delay|effectSend|arp)/.test(n);
const unsupported=fa.filter(n=>!used.has(n)&&!excluded(n));
let output=`/* Generated factory ${bankNames[0]}–${bankNames.at(-1)} programs; original names, mapped native controls. */\n`;
output+=`#define VC_PRESET_BANK_COUNT ${bankCount}u\n#define VC_PRESET_FIRST_BANK '${bankNames[0]}'\n`;
output+=`#define VC_PRESET_COUNT ${count}u\nstatic const uint8_t preset_params[${count}][VC_PARAMS]={\n`+patches.map(({patch:p})=>'{'+ids.map(id=>derived[id]?derived[id](p):p[map[id]]).join(',')+'}').join(',\n')+'\n};\n';
output+=`static const char preset_names[${count}][11]={\n`+patches.map(p=>JSON.stringify(p.name)).join(',\n')+'\n};\n';
output+=`static const uint8_t preset_partial[${count}]={`+patches.map(({patch:p})=>+(p.unisonMode>3||p.fmMode>3||[1,2,3,4,5,6].some(i=>p['assign'+i+'Source']&&[p['assign'+i+'Dest'],p['assign'+i+'Dest1'],p['assign'+i+'Dest2'],p['assign'+i+'Dest3']].some(Boolean))||p.lfo1AssignDest>0||p.lfo2AssignDest>0||p.inputMode>0||p.vocoderMode>0)).join(',')+'};\n';
output+=`static const uint8_t preset_banks[${count}]={`+patches.map(p=>p.bank-firstBank).join(',')+'};\n';
output+=`static const uint8_t preset_programs[${count}]={`+patches.map(p=>p.program).join(',')+'};\n';
fs.writeFileSync(root+'/engine/presets.inc',output);
fs.writeFileSync(root+'/presets/factory-decoded.json',JSON.stringify(patches,null,2));fs.writeFileSync(root+'/presets/parameter-map.json',JSON.stringify({native_order:ids,direct:map,derived:Object.keys(derived)},null,2));
const audit={source:model?{model:`Virus ${model}`,files:filenames.map((f,i)=>({name:path.basename(f),sha256:crypto.createHash('sha256').update(inputs[i]).digest('hex')}))}:{url:'https://virooos.vercel.app/presets/virus-c-rom.syx'},source_sha256:crypto.createHash('sha256').update(raw).digest('hex'),original_programs:allPrograms,excluded_arp_programs:excludedArp.length,programs:count,banks:bankNames,native_parameters:ids.length,mapped_source_parameters:used.size,unconnected:unsupported.map(n=>({parameter:n,nondefault_programs:patches.filter(p=>p.patch[n]!==da[n].def).length})),excluded:fa.filter(excluded)};fs.writeFileSync(root+'/presets/coverage.json',JSON.stringify(audit,null,2));console.log(JSON.stringify(audit));
