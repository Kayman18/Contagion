#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const unsigned dest[]={9,14,17,45,46},modes[]={1,2,4};const float levels[]={-1,0,.37f,1},lfos[]={-1,-.25f,.5f,1};
 for(unsigned d=0;d<5;d++)for(unsigned amount=0;amount<128;amount++)for(unsigned lfo3=0;lfo3<6;lfo3++)for(unsigned mode=0;mode<3;mode++)for(unsigned test=0;test<4;test++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_LFO_OSC1,81);vc_param(&s,VC_LFO_OSC2,47);vc_param(&s,VC_OSC2_ENV,71);vc_param(&s,VC_LFO_MODE,1);
  vc_param(&s,VC_LFO3_ENABLE,1);vc_param(&s,VC_LFO3_FADE,0);vc_param(&s,VC_LFO3_AMOUNT,73);vc_param(&s,VC_LFO3_DEST,lfo3);vc_param(&s,VC_OSC3_MODE,modes[mode]);
  VCMatrix m={0};for(unsigned k=0;k<11;k++)m.route[k].amount=64;m.route[0]=(VCMatrixRoute){3,dest[d],amount};
  vc_set_matrix(&s,&m);vc_note_on(&s,test<2?60:120,101);VCVoice *v=&s.voice[0];
  s.controllers[3]=levels[test];v->lfo.value=lfos[test];v->lfo3.value=-.25f;v->filter_env.level=test/3.f;
  for(unsigned k=0;k<2;k++){v->osc[k].phase=.31f;v->osc[k].pending=.007f;}v->osc3.phase=.17f;v->osc3.pending=.003f;
  vc_filter_control(&s,v);
  for(unsigned k=0;k<2;k++){assert(v->osc[k].phase==.31f&&v->osc[k].pending==.007f);assert(v->osc[k].band==vc_osc_band(v->inc[k]));}
  assert(v->osc3.phase==.17f&&v->osc3.pending==.003f&&v->osc3.band==vc_osc_band(v->inc3));
  printf("%u %u %u %u %u %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",dest[d],amount,lfo3,modes[mode],test,v->pitch1_base_inc,v->pitch2_base_inc,v->osc3_base_inc,v->inc[0],v->inc[1],v->inc3,v->pitch1_last,v->pitch2_env_last,v->osc3_pitch_last);
 }
 /* Independent note velocities produce independent pitch; removing the matrix
  * restores pitch without retriggering or changing phase. */
 VCSynth s;vc_init(&s,44100);vc_note_on(&s,60,20);vc_note_on(&s,60,110);
 float original=s.voice[0].inc[0];VCMatrix m={0};m.route[0]=(VCMatrixRoute){24,9,127};vc_set_matrix(&s,&m);
 vc_filter_control(&s,&s.voice[0]);float first=s.voice[0].inc[0];vc_filter_control(&s,&s.voice[1]);assert(s.voice[0].inc[0]==first&&first!=s.voice[1].inc[0]);
 vc_set_matrix(&s,0);float phase=s.voice[0].osc[0].phase;vc_filter_control(&s,&s.voice[0]);assert(s.voice[0].inc[0]==original&&s.voice[0].osc[0].phase==phase);
 return 0;
}
