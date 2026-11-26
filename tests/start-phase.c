#include "virooos.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
static void emit(float x){fwrite(&x,4,1,stdout);}
int main(void){
 for(unsigned phase=0;phase<=128;phase++)for(unsigned take=0;take<3;take++){
  VCSynth s;vc_init(&s,44100);s.counter=take;vc_param(&s,VC_INIT_PHASE,phase);vc_note_on(&s,60,100);VCVoice *v=&s.voice[0];
  emit(v->osc[0].phase);emit(v->osc[1].phase);emit(v->sub.phase);
  assert(v->osc[0].phase>=0&&v->osc[0].phase<1&&v->osc[1].phase>=0&&v->osc[1].phase<1);
  assert(v->noise.rng==(0x9e3779b9u^(60u*0x85ebca6bu)^((take+1)*0xc2b2ae35u)));
  VCSynth unchanged=s;
  VCOsc a=v->osc[0],b=v->osc[1];VCSub sub=v->sub;int control=v->control_remaining;
  vc_param(&s,VC_INIT_PHASE,phase?0:64);
  assert(!memcmp(&a,&v->osc[0],sizeof(a))&&!memcmp(&b,&v->osc[1],sizeof(b))&&!memcmp(&sub,&v->sub,sizeof(sub))&&control==v->control_remaining);
  float first[256],second[256];vc_render(&s,first,256);vc_render(&unchanged,second,256);assert(!memcmp(first,second,sizeof(first)));
 }
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_INIT_PHASE,999);assert(s.param[VC_INIT_PHASE]==128);
 vc_param(&s,VC_INIT_PHASE,0);float previous=-1;unsigned variations=0;
 for(unsigned n=0;n<256;n++){vc_panic(&s);vc_note_on(&s,60,100);if(s.voice[0].osc[1].phase!=previous)variations++;previous=s.voice[0].osc[1].phase;}
 assert(variations==256);
 for(unsigned phase=0;phase<=128;phase++){
  float a[256],b[256];vc_init(&s,44100);vc_param(&s,VC_SHAPE1,0);vc_param(&s,VC_SHAPE2,0);vc_param(&s,VC_INIT_PHASE,phase);
  vc_note_on(&s,60,100);vc_render(&s,a,256);vc_panic(&s);vc_note_on(&s,60,100);vc_render(&s,b,256);
  if(phase)assert(!memcmp(a,b,sizeof(a)));else assert(memcmp(a,b,sizeof(a)));
 }
 return 0;
}
