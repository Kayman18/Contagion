#include "virooos.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
static void emit(float x){assert(isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 const unsigned triggers[]={0,1,65};
 for(unsigned shape=0;shape<6;shape++)for(unsigned contour=0;contour<128;contour++)for(unsigned env=0;env<2;env++)for(unsigned tr=0;tr<3;tr++){
  VCLfo l;vc_lfo_init(&l,32768,0x12345678);l.phase=floorf(l.phase*65536)/65536;
  vc_lfo_config(&l,5.5f,shape,(int)contour-64,env);vc_lfo_trigger(&l,triggers[tr]);
  for(unsigned n=0;n<512;n++)emit(vc_lfo_tick(&l,32));
  if(env)assert(l.done);
 }
 const unsigned notes[]={24,60,96},amounts[]={0,32,64,80,96,112,127},pw[]={0,64,127};const float values[]={-1,-.25f,0,.5f,1,2};
 for(unsigned n=0;n<3;n++)for(unsigned a=0;a<7;a++)for(unsigned p=0;p<3;p++)for(unsigned value=0;value<6;value++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_PW1,32);vc_param(&s,VC_PW2,96);vc_param(&s,VC_LFO_OSC1,amounts[a]);vc_param(&s,VC_LFO_OSC2,127-amounts[a]);vc_param(&s,VC_LFO_PW,pw[p]);vc_param(&s,VC_OSC2_ENV,80);vc_note_on(&s,notes[n],100);
  VCVoice *v=&s.voice[0];v->filter_env.level=.5f;v->lfo_mod=values[value];vc_pitch_env_control(&s,v);
  emit(v->inc[0]);emit(v->inc[1]);emit(v->osc[0].duty);emit(v->osc[1].duty);
 }
 for(unsigned mode=1;mode<=2;mode++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_LFO_MODE,mode);vc_param(&s,VC_LFO_RATE,96);vc_param(&s,VC_LFO_OSC1,80);vc_param(&s,VC_LFO_OSC2,96);vc_param(&s,VC_LFO_PW,80);vc_param(&s,VC_RELEASE,80);vc_note_on(&s,60,100);
  VCVoice *v=&s.voice[0];v->lfo_deviation=1;vc_param(&s,VC_LFO_RATE,95);vc_param(&s,VC_LFO_RATE,96);
  for(unsigned n=0;n<12000;n++){float l,r;vc_render_stereo(&s,&l,&r,1);emit(v->inc[0]);emit(v->inc[1]);emit(v->osc[0].duty);emit(v->osc[1].duty);emit(v->lfo.value);}
 }
}
