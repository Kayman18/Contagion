#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
static void out(VCVoice *v){emit(v->inc[0]);emit(v->inc[1]);emit(v->osc[0].duty);emit(v->osc[1].duty);}
int main(void){
 const float values[]={-1,-.25f,0,.5f,1};
 for(unsigned dest=0;dest<6;dest++)for(unsigned amount=0;amount<128;amount++)for(unsigned val=0;val<5;val++){
  VCSynth s;vc_init(&s,44100);s.param[VC_LFO3_DEST]=dest;s.param[VC_LFO3_AMOUNT]=amount;s.param[VC_LFO_OSC1]=80;s.param[VC_LFO_OSC2]=90;s.param[VC_LFO_PW]=80;s.param[VC_PW1]=32;s.param[VC_PW2]=96;
  vc_note_on(&s,60,100);VCVoice *v=&s.voice[0];v->lfo_mod=.25f;v->lfo3_mod=values[val];vc_pitch_env_control(&s,v);out(v);
 }
 for(unsigned amount=0;amount<128;amount++){
  float duration=vc_lfo3Fade[amount];const float times[]={0,duration/18,duration/9,duration/2,duration,duration+1};
  for(unsigned t=0;t<6;t++){
   VCSynth s;vc_init(&s,44100);s.param[VC_LFO3_ENABLE]=1;s.param[VC_LFO3_FADE]=amount;vc_note_on(&s,60,100);VCVoice *v=&s.voice[0];unsigned frame=(unsigned)(times[t]*44100);if(frame<32)frame=32;
   v->lfo3_elapsed_frames=frame-32;v->lfo3.value=1;vc_filter_control(&s,v);emit((float)frame);emit(v->lfo3_mod);
  }
 }
 for(unsigned dest=0;dest<6;dest++)for(unsigned clock=0;clock<=5;clock+=5){
  VCSynth s;vc_init(&s,44100);s.param[VC_LFO3_ENABLE]=1;s.param[VC_LFO3_RATE]=96;s.param[VC_LFO3_AMOUNT]=96;s.param[VC_LFO3_DEST]=dest;s.param[VC_LFO3_FADE]=32;s.param[VC_LFO3_CLOCK]=clock;
  s.param[VC_LFO_MODE]=1;s.param[VC_LFO_OSC1]=80;s.param[VC_LFO_PW]=80;vc_note_on(&s,60,100);VCVoice *v=&s.voice[0];v->lfo_deviation=v->lfo3_deviation=1;v->lfo3.phase=.5f;vc_update_voice(&s,v);
  for(unsigned n=0;n<16000;n++){float l,r;vc_render_stereo(&s,&l,&r,1);out(v);emit(v->lfo3.value);emit(v->lfo3_mod);}
 }
}
