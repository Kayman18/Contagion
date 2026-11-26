#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 VCSynth s;vc_init(&s,44100);
 vc_param(&s,VC_F_AMOUNT1,43);vc_param(&s,VC_F_AMOUNT2,91);
 vc_param(&s,VC_VEL_ENV1,47);vc_param(&s,VC_VEL_ENV2,79);
 vc_param(&s,VC_VEL_RES1,43);vc_param(&s,VC_VEL_RES2,99);
 vc_param(&s,VC_F_KEYFOLLOW1,27);vc_param(&s,VC_F_KEYFOLLOW2,99);
 vc_param(&s,VC_LFO_FILTER_GAIN,64);vc_param(&s,VC_LFO_RESONANCE,64);vc_param(&s,VC_LFO2_CUTOFF1,64);vc_param(&s,VC_LFO2_CUTOFF2,64);
 vc_param(&s,VC_CUTOFF,61);vc_param(&s,VC_CUTOFF2,73);
 vc_param(&s,VC_RESONANCE,27);vc_param(&s,VC_RESONANCE2,43);
 vc_param(&s,VC_LFO_MODE,1);vc_param(&s,VC_LFO2_MODE,1);vc_param(&s,VC_LFO3_ENABLE,1);vc_param(&s,VC_LFO3_FADE,0);
 vc_param(&s,VC_FM_AMOUNT,63);vc_param(&s,VC_FM_ENV,43);vc_param(&s,VC_LFO2_FM_AMOUNT,93);vc_param(&s,VC_FM_VELOCITY,81);
 vc_note_on(&s,72,101);VCVoice *v=&s.voice[0];
 for(unsigned p=0;p<650+4*128;p++)for(unsigned test=0;test<4;test++){
  VCMatrix synthetic={0};
  static const unsigned dest[4]={16,18,53,75};
  for(unsigned j=0;j<11;j++)synthetic.route[j].amount=64;
  if(p>=650){synthetic.route[0].source=3;synthetic.route[0].destination=dest[(p-650)/128];synthetic.route[0].amount=(p-650)%128;}
  vc_set_matrix(&s,p<650?vc_matrix_factory(p):&synthetic);s.param[VC_CUTOFF_LINK]=test&1;
  s.param[VC_F_POLARITY1]=(test>>1)&1;s.param[VC_F_POLARITY2]=1-((test>>1)&1);
  v->note=test<2?48:84;v->pitch=v->note;
  for(unsigned i=0;i<28;i++)s.controllers[i]=0;
  const unsigned cc[]={1,2,3,4,6,8,9,11,12,13,14,15,16,64,65,66};
  for(unsigned i=0;i<16;i++)vc_cc(&s,cc[i],(i*11+test*29)%128);
  vc_pressure(&s,31+test*21);s.bend=-.75f+test*.5f;
  v->env.level=.3f;v->filter_env.level=test/3.f;v->release_velocity=.2f;
  v->lfo.value=-.6f;v->lfo2.value=-1+test*2/3.f;v->lfo3.value=-.25f;
  vc_filter_control(&s,v);
  printf("%u %u %.9g\n",p,test,v->fm_depth);
 }
 /* Release velocity follows actual release, rather than pedal deferral. */
 vc_init(&s,44100);vc_note_on(&s,60,100);v=&s.voice[0];assert(v->release_velocity==100/127.f);
 vc_note_off_velocity(&s,60,88);assert(v->release_velocity==88/127.f);
 vc_init(&s,44100);vc_note_on(&s,60,100);v=&s.voice[0];vc_cc(&s,64,127);
 vc_note_off_velocity(&s,60,88);assert(v->release_velocity==100/127.f);
 vc_cc(&s,64,0);assert(v->release_velocity==0);
 vc_init(&s,44100);assert(!s.matrix);assert(!s.controllers[3]);
 return 0;
}
