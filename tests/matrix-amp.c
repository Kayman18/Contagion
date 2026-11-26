#include <math.h>
#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const unsigned dest[]={1,2,80},velocities[]={1,32,100,127},ccs[]={0,64,127};const float levels[]={-1,0,.37f,1};
 for(unsigned d=0;d<3;d++)for(unsigned amount=0;amount<128;amount++)for(unsigned vi=0;vi<4;vi++)for(unsigned li=0;li<4;li++)for(unsigned ci=0;ci<3;ci++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_AMP_VELOCITY,91);vc_param(&s,VC_PATCH_VOLUME,73);vc_cc(&s,7,ccs[ci]);vc_cc(&s,11,96);
  vc_note_on(&s,60,velocities[vi]);VCVoice *v=&s.voice[0];VCMatrix m={0};for(unsigned k=0;k<11;k++)m.route[k].amount=64;m.route[0]=(VCMatrixRoute){3,dest[d],amount};vc_set_matrix(&s,&m);
  for(unsigned step=0;step<2;step++){
   s.controllers[3]=step?0:levels[li];float start=v->amp_gain;v->osc[0].phase=.31f;v->env.level=.4f;vc_filter_control(&s,v);
   assert(v->osc[0].phase==.31f&&v->env.level==.4f);assert(isfinite(v->amp_target));
   for(unsigned n=1;n<=40;n++){float actual=vc_amp_gain_next(v),want=start+(v->amp_target-start)*(n>=32?1:n/32.f);assert(fabsf(actual-want)<.00001f);}
   assert(v->amp_gain==v->amp_target);printf("%u %u %u %u %u %u %.9g\n",dest[d],amount,velocities[vi],li,ccs[ci],step,v->amp_target);
  }
 }
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_AMP_VELOCITY,64);vc_note_on(&s,60,20);vc_note_on(&s,64,110);
 float base=s.voice[0].amp_target;VCMatrix m={0};m.route[0]=(VCMatrixRoute){24,2,127};vc_set_matrix(&s,&m);
 vc_filter_control(&s,&s.voice[0]);float first=s.voice[0].amp_target;vc_filter_control(&s,&s.voice[1]);assert(first==s.voice[0].amp_target&&first!=s.voice[1].amp_target);
 vc_cc(&s,7,0);vc_filter_control(&s,&s.voice[0]);assert(s.voice[0].amp_target==0);
 vc_cc(&s,7,127);vc_filter_control(&s,&s.voice[0]);assert(s.voice[0].amp_target==first);
 vc_set_matrix(&s,0);assert(s.voice[0].amp_target==base);
 /* INIT's legacy velocity setting 128 stays linear for unrelated routes. */
 vc_init(&s,44100);vc_note_on(&s,60,101);base=s.voice[0].amp_target;m.route[0]=(VCMatrixRoute){3,24,100};vc_set_matrix(&s,&m);vc_filter_control(&s,&s.voice[0]);assert(s.voice[0].amp_target==base);
 return 0;
}
