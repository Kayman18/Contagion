#include "../engine/lfo.c"
#include <assert.h>
#include <stdio.h>
#include "wave_cache.h"
static void prepare(unsigned shape){
 if(shape>=46)return;
 assert(vc_wave_begin(2,shape-4));int r;do{r=vc_wave_service(32768);}while(!r);assert(r==1);
 assert(vc_lfo_active_set(shape%3,shape,vc_wave_table(shape-4,0)));
}
int main(void){
 /* Exact table knots first, then source interpolation and contour. */
 assert(vc_wave_init(vc_wave_seed(),vc_wave_seed_size()));vc_lfo_active_reset();
 assert(!vc_lfo_active_set(3,6,0));assert(!vc_lfo_active_set(0,68,0));assert(!vc_lfo_active_set(0,6,0));
 for(unsigned shape=6;shape<46;shape++){prepare(shape);fwrite(lfo_active_tables[shape%3],8196,1,stdout);}
#ifdef VC_DYNAMIC_WAVES
 static float extra[22*2049];FILE *f=fopen("data/VIROOOS-LFO-EXTRA.bin","rb");assert(f);
 assert(fread(extra,sizeof(extra),1,f)==1);fclose(f);vc_lfo_set_extra_tables(extra);
 fwrite(extra,sizeof(extra),1,stdout);
#endif
 for(unsigned shape=6;shape<=VC_LFO_MAX_SHAPE;shape++){prepare(shape);for(int contour=-64;contour<=63;contour++){
  VCLfo l;vc_lfo_init(&l,32768,123);vc_lfo_config(&l,3.25f,shape,contour,0);
  for(unsigned k=0;k<=512;k++){float phase=k/512.f;float x=vc_lfo_shape(&l,phase);assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
 }
 }
 for(unsigned shape=6;shape<=VC_LFO_MAX_SHAPE;shape++){prepare(shape);for(unsigned env=0;env<=1;env++){
  VCLfo l;vc_lfo_init(&l,32768,123);vc_lfo_config(&l,5.5f,shape,48,env);vc_lfo_trigger(&l,65);
  for(unsigned k=0;k<512;k++){float x=vc_lfo_tick(&l,32);assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
  if(env)assert(l.done);
 }
 }
 return 0;
}
