#include "virooos.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#ifdef VC_DYNAMIC_WAVES
#include "wave_cache.h"
#endif
int main(void){
 const int harmonics[]={128,96,64,48,32,24,16,12,8,6,4,3,2,1};
 const int shapes[]={0,32,63};
 #ifdef VC_DYNAMIC_WAVES
 assert(vc_wave_init(vc_wave_seed(),vc_wave_seed_size()));
#endif
 for(int w=0;w<VC_WAVES;w++){
#ifdef VC_DYNAMIC_WAVES
 assert(vc_wave_begin(0,w));int state=0;while(!state)state=vc_wave_service(4096);assert(state==1);
#endif
 for(int b=0;b<14;b++)for(int s=0;s<3;s++)for(int sync=0;sync<2;sync++){
  VCOsc o={0};float inc=floorf((.49f/harmonics[b])*1048576)/1048576;vc_osc_setup(&o,w,shapes[s],.31f,inc);
  assert(o.wave==w&&o.band==b);
  for(int n=0;n<1024;n++){float x=vc_osc_next(&o,inc,sync&&n%31==0?.375f:-1);assert(isfinite(x));fwrite(&x,4,1,stdout);}
 }
 }
 VCOsc o={0};vc_osc_setup(&o,-1,0,.5f,.01f);assert(o.wave==0);
 vc_osc_setup(&o,100,0,.5f,.01f);assert(o.wave==VC_WAVES-1);
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_WAVE1,127);assert(s.param[VC_WAVE1]==VC_WAVES-1);
 return 0;
}
