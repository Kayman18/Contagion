#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
static void emit(VCEnvelope *e){printf(" %.9g %.9g %.9g %.9g %d %.9g %.9g",e->attack_inc,e->decay_coef,e->sustain,e->release_coef,e->slope,e->fall_coef,e->rise_inc);}
int main(void){
 const unsigned bases[]={0,32,64,127};const float levels[]={-1,0,.37f,1};
 for(unsigned dest=33;dest<=42;dest++)for(unsigned amount=0;amount<128;amount++)for(unsigned b=0;b<4;b++)for(unsigned t=0;t<4;t++){
  VCSynth s;float sr=t&1?48000:44100;vc_init(&s,sr);
  for(unsigned k=0;k<10;k++)vc_param(&s,vc_envelope_ids[k],bases[b]);
  VCMatrix matrix={0};matrix.route[0]=(VCMatrixRoute){3,dest,amount};vc_set_matrix(&s,&matrix);vc_note_on(&s,60,100);s.controllers[3]=levels[t];
  VCVoice *v=&s.voice[0];v->env.stage=v->filter_env.stage=t+1;v->env.level=v->filter_env.level=.375f;
  vc_filter_control(&s,v);
  assert(v->env.stage==(int)t+1&&v->filter_env.stage==(int)t+1&&v->env.level==.375f&&v->filter_env.level==.375f);
  printf("%u %u %u %u %.0f",dest,amount,bases[b],t,sr);emit(&v->filter_env);emit(&v->env);
  for(unsigned i=0;i<16;i++)printf(" %.9g %.9g",vc_env_next(&v->filter_env),vc_env_next(&v->env));
  puts("");
 }
 VCSynth s;vc_init(&s,44100);vc_note_on(&s,60,20);vc_note_on(&s,64,110);
 VCEnvelope original=s.voice[0].env;VCMatrix m={0};m.route[0]=(VCMatrixRoute){24,38,127};vc_set_matrix(&s,&m);
 for(unsigned i=0;i<2;i++)vc_filter_control(&s,&s.voice[i]);
 assert(s.voice[0].env.attack_inc!=s.voice[1].env.attack_inc);
 vc_set_matrix(&s,0);assert(s.voice[0].env.attack_inc==original.attack_inc);
 vc_set_matrix(&s,&m);VCSynth b=s;float l[256],r[256],ll[256],rr[256];vc_render_stereo(&s,l,r,256);
 for(unsigned j=0;j<256;j++)vc_render_stereo(&b,ll+j,rr+j,1);
 assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));
}
