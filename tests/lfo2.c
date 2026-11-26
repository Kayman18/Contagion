#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 const float values[]={-1,-.25f,0,.5f,1,2};const unsigned notes[]={24,60,96};
 for(unsigned link=0;link<2;link++)for(unsigned note=0;note<3;note++)for(unsigned amount=0;amount<128;amount++)for(unsigned j=0;j<6;j++){
  VCSynth s;vc_init(&s,44100);s.param[VC_CUTOFF]=73;s.param[VC_CUTOFF2]=91;s.param[VC_CUTOFF_LINK]=link;
  s.param[VC_F_AMOUNT1]=70;s.param[VC_F_AMOUNT2]=81;s.param[VC_F_KEYFOLLOW1]=90;s.param[VC_F_KEYFOLLOW2]=33;
  s.param[VC_LFO2_CUTOFF1]=amount;s.param[VC_LFO2_CUTOFF2]=127-amount;s.param[VC_LFO2_MODE]=1;
  vc_note_on(&s,notes[note],100);VCVoice *v=&s.voice[0];v->filter_env.level=.3f;v->lfo2.value=values[j];vc_filter_control(&s,v);
  emit(v->filter[0].cutoff);emit(v->filter[1].cutoff);
  s.param[VC_LFO2_MODE]=0;vc_filter_control(&s,v);assert(v->lfo2_mod==0);
 }
 for(unsigned shape=0;shape<4;shape++)for(unsigned mode=1;mode<=2;mode++){
  VCSynth s;vc_init(&s,44100);s.param[VC_CUTOFF]=55;s.param[VC_CUTOFF2]=75;
  s.param[VC_LFO2_MODE]=mode;s.param[VC_LFO2_SHAPE]=shape;s.param[VC_LFO2_RATE]=96;s.param[VC_LFO2_CUTOFF1]=96;s.param[VC_LFO2_CUTOFF2]=32;
  s.param[VC_LFO_MODE]=1;s.param[VC_LFO_OSC1]=80;s.param[VC_LFO_RESONANCE]=80;
  vc_note_on(&s,60,100);VCVoice *v=&s.voice[0];v->lfo_deviation=v->lfo2_deviation=1;vc_update_voice(&s,v);
  for(unsigned i=0;i<12000;i++){
   float l,r;vc_render_stereo(&s,&l,&r,1);emit(v->lfo2.value);emit(v->filter[0].cutoff);emit(v->filter[1].cutoff);emit(v->filter[0].g/(1+v->filter[0].g));emit(v->filter[1].g/(1+v->filter[1].g));emit(v->lfo.value);emit(v->inc[0]);
  }
 }
 /* New random LFO state must not disturb the established random streams. */
 VCSynth a,b;vc_init(&a,44100);vc_init(&b,44100);b.param[VC_LFO2_MODE]=1;b.param[VC_LFO2_SHAPE]=5;vc_note_on(&a,60,100);vc_note_on(&b,60,100);
 assert(a.voice[0].noise.rng==b.voice[0].noise.rng&&a.voice[0].lfo.rng==b.voice[0].lfo.rng);
 assert(a.voice[0].lfo.rng!=a.voice[0].lfo2.rng);
}
