#include "virooos.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>
static void emit(float x){assert(isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 for(int w=0;w<2;w++)for(int shape=0;shape<4;shape++){
 VCOsc o={0};float inc=.0078125f;vc_osc_setup(&o,w,shape==0?0:shape==1?32:shape==2?64:127,.5f,inc);
 for(int i=0;i<2048;i++)emit(vc_osc_next(&o,inc,-1));}
 /* hard sync at fractional sample boundaries */
 {VCOsc a={0},b={0};vc_osc_setup(&a,0,64,.5f,.015625f);vc_osc_setup(&b,0,127,.37f,.0234375f);
 for(int i=0;i<2048;i++){float sync=a.phase+.015625f>=1?(1-a.phase)/.015625f:-1;vc_osc_next(&a,.015625f,-1);emit(vc_osc_next(&b,.0234375f,sync));}}
 for(int mode=0;mode<4;mode++)for(int poles=2;poles<=4;poles+=2)for(int res=0;res<=127;res+=127){VCFilter f={0};vc_filter_setup(&f,44100,mode,poles,80,res);for(int i=0;i<4096;i++)emit(vc_filter_next(&f,i==0?1:0));}
 {VCEnvelope e={0};vc_env_setup(&e,44100,.01f,.04f,.5f,.03f);e.stage=1;for(int i=0;i<20000;i++){if(i==10000)e.stage=4;emit(vc_env_next(&e));}}
 return 0;
}
