#include "virooos.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
static void emit(float x){assert(isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 const unsigned mods[]={0,64,127},amounts[]={0,1,32,64,96,127},shapes[]={0,32,64,127},colors[]={0,64,127};
 for(unsigned a=0;a<128;a++)for(unsigned e=0;e<3;e++)for(unsigned vel=0;vel<3;vel++)for(unsigned level=0;level<3;level++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_FM_AMOUNT,a);vc_param(&s,VC_FM_ENV,mods[e]);vc_param(&s,VC_FM_VELOCITY,mods[vel]);vc_note_on(&s,60,100);
  VCVoice *v=&s.voice[0];v->filter_env.level=level*.5f;vc_fm_control(&s,v);emit(v->fm_depth);
 }
 for(unsigned mode=0;mode<4;mode++)for(unsigned a=0;a<6;a++)for(unsigned sh=0;sh<4;sh++)for(unsigned c=0;c<3;c++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_FM_MODE,mode);vc_param(&s,VC_FM_AMOUNT,amounts[a]);vc_param(&s,VC_SHAPE1,shapes[sh]);vc_param(&s,VC_NOISE_COLOR,colors[c]);vc_note_on(&s,60,100);
  VCVoice *v=&s.voice[0];v->inc[1]=3.f/256;v->noise.rng=0x12345678;
  for(unsigned n=0;n<64;n++){
   if(v->fm_depth>0&&mode==3)v->fm_noise_sample=vc_noise_next(&v->noise);
   emit(v->fm_depth>0?vc_fm_increment(v,mode,n/64.f,((int)(n%17)-8)/8.f):v->inc[1]);
  }
 }
 return 0;
}
