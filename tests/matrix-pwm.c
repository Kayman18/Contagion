#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const unsigned dest[]={7,12,47,74};const float levels[]={-1,0,.37f,1};
 for(unsigned d=0;d<4;d++)for(unsigned amount=0;amount<128;amount++)for(unsigned lfo3=0;lfo3<6;lfo3++)for(unsigned test=0;test<4;test++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_PW1,21);vc_param(&s,VC_PW2,93);
  vc_param(&s,VC_VEL_PW,47);vc_param(&s,VC_LFO_PW,81);vc_param(&s,VC_LFO_MODE,1);
  vc_param(&s,VC_LFO3_ENABLE,1);vc_param(&s,VC_LFO3_FADE,0);vc_param(&s,VC_LFO3_AMOUNT,73);vc_param(&s,VC_LFO3_DEST,lfo3);vc_param(&s,VC_OSC3_MODE,1);
  VCMatrix m={0};for(unsigned k=0;k<11;k++)m.route[k].amount=64;m.route[0]=(VCMatrixRoute){3,dest[d],amount};
  vc_set_matrix(&s,&m);vc_note_on(&s,60,101);VCVoice *v=&s.voice[0];
  for(unsigned step=0;step<2;step++){
   s.controllers[3]=step?0:levels[test];v->lfo.value=-1+test*2/3.f;v->lfo3.value=-.25f;
   for(unsigned k=0;k<2;k++){v->osc[k].phase=.31f;v->osc[k].pending=.007f;}
   v->osc3.phase=.17f;v->osc3.pending=.003f;
   vc_filter_control(&s,v);
   for(unsigned k=0;k<2;k++)assert(v->osc[k].phase==.31f&&v->osc[k].pending==.007f);
   assert(v->osc3.phase==.17f&&v->osc3.pending==.003f);
   printf("%u %u %u %u %u %.9g %.9g %.9g\n",dest[d],amount,lfo3,test,step,v->osc[0].duty,v->osc[1].duty,v->osc3.duty);
  }
 }
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_PW1,21);vc_note_on(&s,60,20);vc_note_on(&s,64,110);
 float original=s.voice[0].osc[0].duty;VCMatrix m={0};m.route[0]=(VCMatrixRoute){24,7,127};vc_set_matrix(&s,&m);
 vc_filter_control(&s,&s.voice[0]);float first=s.voice[0].osc[0].duty;vc_filter_control(&s,&s.voice[1]);assert(first==s.voice[0].osc[0].duty&&first!=s.voice[1].osc[0].duty);
 vc_set_matrix(&s,0);vc_filter_control(&s,&s.voice[0]);assert(s.voice[0].osc[0].duty==original);
 return 0;
}
