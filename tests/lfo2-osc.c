#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
static void gains(VCVoice *v){for(unsigned j=0;j<2;j++){emit(v->osc[j].wave_gain);emit(v->osc[j].saw_gain);emit(v->osc[j].pulse_gain);}emit(v->fm_depth);emit(v->fm_scale);}
int main(void){
 const unsigned shapes[]={0,32,63,64,80,127};const float values[]={-1,-.25f,0,.5f,1,2};
 for(unsigned a=0;a<128;a++)for(unsigned sh=0;sh<6;sh++)for(unsigned val=0;val<6;val++){
  VCSynth s;vc_init(&s,44100);s.param[VC_SHAPE1]=shapes[sh];s.param[VC_SHAPE2]=127-shapes[sh];s.param[VC_LFO2_SHAPE_AMOUNT]=a;s.param[VC_LFO2_FM_AMOUNT]=127-a;
  s.param[VC_FM_AMOUNT]=32;s.param[VC_FM_ENV]=80;s.param[VC_FM_VELOCITY]=70;vc_note_on(&s,60,100);
  VCVoice *v=&s.voice[0];v->filter_env.level=.5f;v->lfo2_mod=values[val];v->osc[0].phase=.375f;v->osc[0].pending=.125f;
  float duty=v->osc[0].duty;const float *table=v->osc[0].table;
  vc_lfo2_osc_control(&s,v);vc_fm_control(&s,v);gains(v);
  assert(v->osc[0].phase==.375f&&v->osc[0].pending==.125f&&v->osc[0].duty==duty&&v->osc[0].table==table);
  v->lfo2_mod=0;vc_lfo2_osc_control(&s,v);vc_fm_control(&s,v);gains(v);
 }
 const unsigned clocks[]={0,5,17};
 for(unsigned c=0;c<3;c++)for(unsigned mode=1;mode<=2;mode++){
  VCSynth s;vc_init(&s,44100);s.param[VC_SHAPE1]=32;s.param[VC_SHAPE2]=96;s.param[VC_LFO2_RATE]=96;s.param[VC_LFO2_MODE]=mode;s.param[VC_LFO2_CLOCK]=clocks[c];s.param[VC_LFO2_SHAPE_AMOUNT]=112;s.param[VC_LFO2_FM_AMOUNT]=80;s.param[VC_FM_AMOUNT]=16;
  s.param[VC_LFO_MODE]=1;s.param[VC_LFO_OSC1]=80;s.param[VC_LFO_PW]=80;
  vc_note_on(&s,60,100);VCVoice *v=&s.voice[0];v->lfo_deviation=v->lfo2_deviation=1;vc_update_voice(&s,v);
  for(unsigned n=0;n<10000;n++){float l,r;vc_render_stereo(&s,&l,&r,1);gains(v);emit(v->lfo2.rate);emit(v->lfo2.value);emit(v->osc[0].duty);}
 }
}
