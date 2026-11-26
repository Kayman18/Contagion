const assert=require('assert'),r=require('../tools/reference.cjs'),presets=require('../presets/factory-decoded.json');let checks=0;
for(const {patch:p}of presets){const q={...p};let changed=false;
 for(const[, ...pairs]of r.matrixSlots)for(const[d]of pairs)if(p[d]&&!r.matrixDestinations[p[d]]?.[1]){q[d]=0;changed=true;}
 for(const[,d]of r.matrixLfos)if(p[d]&&!r.matrixDestinations[p[d]]?.[1]){q[d]=0;changed=true;}
 if(!changed)continue;
 for(const level of [-1,0,.5,1]){const sources=Array(28).fill(level),a={...p},b={...p};new r.Matrix().apply(p,sources,a,[]);new r.Matrix().apply(q,sources,b,[]);assert.deepStrictEqual(a,b);checks++;}
}
console.log(JSON.stringify({sourceInactiveRemovalChecks:checks,unchangedOriginalMatrixOutputs:true,pass:true}));
