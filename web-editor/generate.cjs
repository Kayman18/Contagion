const fs = require('fs');
const path = require('path');
const root = path.resolve(process.argv[2] || path.join(__dirname,'..')) + '/';
const read = p => fs.readFileSync(root + p, 'utf8');
const app = read('app/app.c'), engine = read('engine/virooos.c'), header = read('engine/virooos.h');
const ids = header.slice(header.indexOf('VC_SHAPE1'), header.indexOf('VC_PARAMS')).match(/VC_[A-Z0-9_]+/g);
const labels = [...app.match(/\blabels\[VC_PARAMS\]\s*=\s*\{([\s\S]*?)\};/)[1].matchAll(/"([^"]*)"/g)].map(m => m[1]);
if(ids.length!==126 || labels.length!==126) throw Error('Parameter schema changed');
const max = {};
for(const [,cases,value] of engine.slice(engine.indexOf('unsigned vc_param_max'),engine.indexOf('void vc_param(')).matchAll(/((?:case VC_[A-Z0-9_]+:\s*)+)return ([^;]+);/g)) {
  const limits = {'VC_LFO_MAX_SHAPE':67,'VC_WAVES+3':67,'VC_WAVES-1':63};
  for(const id of cases.match(/VC_[A-Z0-9_]+/g)) max[id] = limits[value] ?? Number(value);
}
const live = {VC_WAVE1:19,VC_WAVE2:24};
for(const [,cc,id] of engine.slice(engine.indexOf('int vc_cc_param'),engine.indexOf('void vc_cc(')).matchAll(/case (\d+):return (VC_[A-Z0-9_]+);/g)) live[id] ??= Number(cc);
const presets = read('engine/presets.inc'), matrix = read('engine/matrix.inc');
function body(text,key) { return text.match(new RegExp('\\b'+key+'\\b[^=]*=\\s*\\{([\\s\\S]*?)\\};'))[1]; }
const nums = (text,key) => body(text,key).match(/\d+/g).map(Number);
const names = JSON.parse('[' + body(presets,'preset_names') + ']');
const banks = nums(presets,'preset_banks'), programs = nums(presets,'preset_programs');
const params = nums(presets,'preset_params'), routes = nums(matrix,'matrix_factory');
if(names.length!==1525 || params.length!==names.length*126 || routes.length!==names.length*33) throw Error('Factory build mismatch');
const ref = require(root + 'tools/reference.cjs');
const matrixSources = [];
for(const [name,id] of Object.entries(ref.sourceIds)) matrixSources[id]=name;
const data = {
  matrixSources, matrixTargets: ref.matrixDestinations.map(([name])=>name),
  controls: ids.map((id,i) => [id.slice(3),labels[i],max[id]??127,live[id]??null]),
  factoryIndex: names.map((n,i) => [banks[i],programs[i],n]),
  parameters: Buffer.from(params.map((value, i) => Math.min(value, max[ids[i % ids.length]] ?? 127))).toString('base64'),
  routes: Buffer.from(routes).toString('base64')
};
const html = fs.readFileSync(path.join(__dirname,'editor.html'),'utf8');
fs.writeFileSync(path.join(__dirname,'Contagion-USB-Editor.html'),html.replace('/*__BUILD_DATA__*/',()=>JSON.stringify(data).replaceAll('<','\\u003c')));
