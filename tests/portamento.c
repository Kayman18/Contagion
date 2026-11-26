#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 for(unsigned i=0;i<128;i++){emit(vc_portamento[i]);emit(vc_glideCoef[i]);}
 const unsigned amounts[]={1,32,64,96,127};
 for(unsigned amount=0;amount<5;amount++)for(unsigned dir=0;dir<2;dir++)for(unsigned mode=1;mode<=4;mode++){
  unsigned from=dir?84:36,to=dir?36:84;VCSynth s;vc_init(&s,44100);s.last_note=from;s.param[VC_PORTAMENTO]=amounts[amount];s.param[VC_OSC3_MODE]=mode;s.param[VC_F_KEYFOLLOW1]=96;s.param[VC_CUTOFF]=50;s.param[VC_SAT_CURVE]=12;s.param[VC_SAT_DRIVE]=96;vc_note_on(&s,to,100);VCVoice *v=&s.voice[0];
  for(unsigned tick=0;tick<5000;tick++){
   vc_filter_control(&s,v);emit(v->pitch);emit(v->inc[0]);emit(v->inc[1]);emit(v->inc3);emit(v->filter[0].cutoff);emit(v->saturation.coef);
  }
  float level=v->env.level;vc_param(&s,VC_PORTAMENTO,0);assert(v->pitch==to&&v->env.level==level);
 }
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_PORTAMENTO,127);vc_note_on(&s,36,100);assert(s.voice[0].pitch==36);vc_note_on(&s,84,100);VCVoice *v=&s.voice[1];assert(v->pitch==36);
 for(unsigned tick=0;tick<100000&&v->pitch!=84;tick++)vc_filter_control(&s,v);
 assert(v->pitch==84);vc_panic(&s);assert(s.last_note==84);vc_note_on(&s,60,100);assert(s.voice[0].pitch==84);
}
