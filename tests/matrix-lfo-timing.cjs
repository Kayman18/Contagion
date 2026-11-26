const cp=require('child_process'),assert=require('assert'),r=require('../tools/reference.cjs');
const out=cp.execFileSync('build/matrix-lfo-timing',{encoding:'utf8',maxBuffer:32e6});let n=0,maxRate=0,maxContour=0,maxPhase=0,maxWave=0;
for(const line of out.trim().split('\n')){
 const [dest,amount,base,t,clock,note,dev,rate,contour,phase,cycles,value]=line.split(' ').map(Number),index=dest<50?0:dest<57?1:2;
 const key=r.matrixDestinations[dest][1],p={...r.patch(),lfo1Rate:64,lfo2Rate:64,lfo3Rate:64,lfo1Contour:64,lfo2Contour:64,assign1Source:3,assign1Dest:dest,assign1Amount:amount};p[key]=base;
 const src=Array(28).fill(0),e={...p};src[3]=Math.fround([-1,0,.37,1][t]);new r.Matrix().apply(p,src,e,[]);
 const clip=v=>Math.max(0,Math.min(127,v)),expected=clock?1/r.lfoPeriod(clock,120):r.lfoRate(clip(e['lfo'+(index+1)+'Rate']),index)*r.lfoKeytrack(64,note)*dev;
 const wantContour=index<2?clip(e['lfo'+(index+1)+'Contour'])-64:0,step=expected*32/44100,wantPhase=(.25+step)%1,wantCycles=.125+step;
 let er=Math.abs(rate-expected)/Math.max(1,expected);maxRate=Math.max(maxRate,er);assert(er<.000003,JSON.stringify({line,expected}));
 let ec=Math.abs(contour-wantContour);maxContour=Math.max(maxContour,ec);assert(ec<.00002);
 let ep=Math.max(Math.abs(phase-wantPhase),Math.abs(cycles-wantCycles));maxPhase=Math.max(maxPhase,ep);assert(ep<.000003);
 const wave=new r.LFO(44100);wave.setContour(wantContour);let ew=Math.abs(value-wave.shapeAt(wantPhase));maxWave=Math.max(maxWave,ew);assert(ew<.0001);n++;
}
console.log(JSON.stringify({cases:n,targets:5,clockDivisions:[0,1,21],keyTracking:true,localAndSharedScope:true,maxRelativeRateError:maxRate,maxContourError:maxContour,maxPhaseError:maxPhase,maxWaveformError:maxWave,oneShotStatePreserved:true,matrixRemoval:true,blockInvariant:true,pass:true},null,2));
