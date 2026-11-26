#include "virooos.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
static void emit(float x){assert(isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 const int notes[]={0,36,60,84,127},tunes[]={0,16,52,64,71,112,127},follow[]={0,32,64,80,96,112,127},widths[]={0,32,96,127},bends[]={0,8192,12288};
 for(int n=0;n<5;n++)for(int t=0;t<7;t++)for(int k=0;k<7;k++)for(int w=0;w<4;w++)for(int b=0;b<3;b++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_TUNE1,tunes[t]);vc_param(&s,VC_TUNE2,127-tunes[t]);
  vc_param(&s,VC_KEYFOLLOW1,follow[k]);vc_param(&s,VC_KEYFOLLOW2,127-follow[k]);
  vc_param(&s,VC_PW1,widths[w]);vc_param(&s,VC_PW2,127-widths[w]);vc_bend(&s,bends[b]);vc_note_on(&s,notes[n],100);
  VCVoice *v=&s.voice[0];emit(v->inc[0]);emit(v->inc[1]);emit(v->osc[0].duty);emit(v->osc[1].duty);
 }
 return 0;
}
