#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const unsigned bases[]={0,32,64,127};const float levels[]={-1,0,.003f,.37f,1};
 for(unsigned a=0;a<128;a++)for(unsigned b=0;b<4;b++)for(unsigned t=0;t<5;t++)for(unsigned rate=0;rate<2;rate++){
  VCSynth s;float sr=rate?48000:44100;vc_init(&s,sr);vc_param(&s,VC_PORTAMENTO,bases[b]);
  VCMatrix matrix={0};matrix.route[0]=(VCMatrixRoute){3,5,a};vc_set_matrix(&s,&matrix);vc_note_on(&s,84,100);
  VCVoice *v=&s.voice[0];v->pitch=48;v->glide_error=0;v->osc[0].phase=.31f;v->osc[0].pending=.007f;s.controllers[3]=levels[t];
  vc_filter_control(&s,v);float first=v->pitch;
  assert(v->osc[0].phase==.31f&&v->osc[0].pending==.007f&&v->env.stage==1);
  for(unsigned k=0;k<32;k++)vc_glide_control(&s,v);
  printf("%u %u %u %.0f %.9g %.9g %.9g %.9g\n",a,bases[b],t,sr,v->portamento_value,v->portamento_coef,first,v->pitch);
 }
 for(unsigned mode=1;mode<=4;mode++)for(unsigned count=1;count<=4;count++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_KEY_MODE,mode);vc_param(&s,VC_UNISON_MODE,count-1);
  VCMatrix m={0};m.route[0]=(VCMatrixRoute){3,5,127};vc_set_matrix(&s,&m);s.controllers[3]=1;vc_note_on(&s,60,100);
  for(unsigned i=0;i<count;i++)vc_filter_control(&s,&s.voice[i]);
  vc_note_on(&s,72,100);
  for(unsigned i=0;i<count;i++){assert(s.voice[i].pitch==60&&s.voice[i].portamento_value==126);vc_filter_control(&s,&s.voice[i]);assert(s.voice[i].pitch>60&&s.voice[i].pitch<72);}
  s.controllers[3]=0;
  for(unsigned i=0;i<count;i++){vc_filter_control(&s,&s.voice[i]);assert(s.voice[i].pitch<72);vc_filter_control(&s,&s.voice[i]);assert(s.voice[i].pitch==72);}
 }
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_PORTAMENTO,64);vc_note_on(&s,60,20);vc_note_on(&s,72,110);
 VCMatrix m={0};m.route[0]=(VCMatrixRoute){24,5,127};vc_set_matrix(&s,&m);
 for(unsigned i=0;i<2;i++)vc_filter_control(&s,&s.voice[i]);
 assert(s.voice[0].portamento_coef!=s.voice[1].portamento_coef);
 float old[2]={s.voice[0].portamento_coef,s.voice[1].portamento_coef};
 vc_bend(&s,12288);for(unsigned i=0;i<2;i++)assert(s.voice[i].portamento_coef==old[i]);
 vc_set_matrix(&s,&m);for(unsigned i=0;i<2;i++)assert(s.voice[i].portamento_value==64);
 for(unsigned i=0;i<2;i++)vc_filter_control(&s,&s.voice[i]);
 vc_set_matrix(&s,0);for(unsigned i=0;i<2;i++)assert(s.voice[i].portamento_value==64&&s.voice[i].portamento_coef==vc_glideCoef[64]);
 vc_set_matrix(&s,&m);VCSynth b=s;float l[256],r[256],ll[256],rr[256];vc_render_stereo(&s,l,r,256);
 for(unsigned j=0;j<256;j++)vc_render_stereo(&b,ll+j,rr+j,1);
 assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));
}
