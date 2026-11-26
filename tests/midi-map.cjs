const r=require('../tools/reference.cjs'),map=require('./midi-map.json');
for(const row of map)if(r.ccMap[row.cc]!==row.source)throw Error(JSON.stringify(row));
console.log(`${map.length} original-source CC mappings verified; CC71/74 retain native aliases`);
for(const [cc,name] of [[19,'osc1Wave'],[24,'osc2Wave']])if(r.ccMap[cc]!==name)throw Error('wave CC map mismatch');
console.log('Original CC19/24 verified; handled only by the app muted loader');
if(r.ccMap[94]!=='keyMode')throw Error('key mode CC mismatch');
console.log('Original CC94 keyMode verified; native range 0-5, including HOLD');
