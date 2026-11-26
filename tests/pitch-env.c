#include "virooos.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
static void emit(float x){assert(isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 const int notes[]={0,24,60,96,127};
 for(unsigned note=0;note<5;note++)for(unsigned amount=0;amount<128;amount++)for(unsigned level=0;level<4;level++)for(unsigned bend=0;bend<3;bend++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_OSC2_ENV,amount);vc_bend(&s,4096+bend*4096);vc_note_on(&s,notes[note],100);
  VCVoice *v=&s.voice[0];float inc1=v->inc[0];v->filter_env.level=level==3?1:level*.25f;v->osc[1].phase=.375f;
  vc_pitch_env_control(&s,v);assert(v->osc[1].phase==.375f&&v->inc[0]==inc1);
  emit(v->inc[1]);emit((float)v->osc[1].band);
 }
 const unsigned amounts[]={0,80,127};
 for(unsigned a=0;a<3;a++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_OSC2_ENV,amounts[a]);vc_param(&s,VC_F_ATTACK,12);vc_param(&s,VC_F_DECAY,55);vc_param(&s,VC_F_SUSTAIN,32);vc_param(&s,VC_F_RELEASE,44);vc_param(&s,VC_RELEASE,80);vc_note_on(&s,60,100);
  for(unsigned n=0;n<6000;n++){float l,r;if(n==3000)vc_note_off(&s,60);vc_render_stereo(&s,&l,&r,1);emit(s.voice[0].filter_env.level);emit(s.voice[0].inc[1]);}
 }
}
