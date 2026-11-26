#include <math.h>
#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const unsigned dest[]={3,56,60,81};const float levels[]={-1,0,.37f,1};
 for(unsigned route=0;route<4;route++)for(unsigned d=0;d<4;d++)for(unsigned amount=0;amount<128;amount++)for(unsigned j=0;j<4;j++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_ROUTING,route);vc_param(&s,VC_PAN,59);vc_param(&s,VC_SPREAD,87);
  vc_param(&s,VC_VEL_PAN,91);vc_param(&s,VC_LFO2_PAN,37);vc_param(&s,VC_LFO2_MODE,1);
  VCMatrix m={0};for(unsigned i=0;i<11;i++)m.route[i].amount=64;
  m.route[0]=(VCMatrixRoute){3,dest[d],amount};vc_set_matrix(&s,&m);s.controllers[3]=levels[j];
  vc_note_on(&s,60,101);VCVoice *v=&s.voice[0];v->lfo2.value=.4f;vc_filter_control(&s,v);
  printf("%u %u %u %.9g",route,dest[d],amount,levels[j]);for(unsigned i=0;i<4;i++)printf(" %.9g",v->pan_target[i]);puts("");
  /* Confirm a controller move glides from the current gains to the target
   * over exactly 32 samples, then remains stable. */
  s.controllers[3]=-levels[j];v->lfo2.value=.4f;float start[4];for(unsigned i=0;i<4;i++)start[i]=v->pan_now[i];
  vc_filter_control(&s,v);
  for(unsigned n=1;n<=40;n++){if(n==33){v->lfo2.value=.4f;vc_filter_control(&s,v);}float l,r;vc_route_sample(&s,v,.1f,-.07f,&l,&r);assert(isfinite(l)&&isfinite(r));
   for(unsigned i=0;i<4;i++){float want=n>=32?v->pan_target[i]:start[i]+(v->pan_target[i]-start[i])*n/32;if(fabsf(v->pan_now[i]-want)>=.000002f){fprintf(stderr,"route %u dest %u amount %u level %u sample %u gain %u actual %.9g want %.9g\n",route,d,amount,j,n,i,v->pan_now[i],want);return 1;}}}
 }
 /* Clearing the matrix smoothly returns to the original centred static pan. */
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_PAN,64);vc_param(&s,VC_VEL_PAN,64);vc_param(&s,VC_LFO2_PAN,64);
 VCMatrix m={0};m.route[0]=(VCMatrixRoute){24,3,127};vc_set_matrix(&s,&m);
 vc_note_on(&s,60,20);vc_note_on(&s,64,110);
 vc_filter_control(&s,&s.voice[0]);vc_filter_control(&s,&s.voice[1]);assert(s.voice[0].pan_target[0]!=s.voice[1].pan_target[0]);
 vc_set_matrix(&s,0);
 for(unsigned k=0;k<2;k++){VCVoice *v=&s.voice[k];vc_filter_control(&s,v);for(unsigned i=0;i<32;i++){float l,r;vc_route_sample(&s,v,.1f,.1f,&l,&r);}assert(!v->pan_active);for(unsigned i=0;i<4;i++)assert(v->pan_now[i]==s.pan[i]);}
 return 0;
}
