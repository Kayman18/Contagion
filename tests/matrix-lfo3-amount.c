#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const unsigned bases[]={0,64,125,127},modes[]={1,2,4};const float src[]={-1,0,.37f,1},lfo[]={-1,-.25f,.5f,1};
 for(unsigned dest=0;dest<6;dest++)for(unsigned amount=0;amount<128;amount++)for(unsigned b=0;b<4;b++)for(unsigned t=0;t<4;t++)for(unsigned m=0;m<3;m++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_LFO3_ENABLE,1);vc_param(&s,VC_LFO3_AMOUNT,bases[b]);vc_param(&s,VC_LFO3_FADE,0);vc_param(&s,VC_LFO3_DEST,dest);vc_param(&s,VC_LFO3_GLOBAL,t&1);vc_param(&s,VC_OSC3_MODE,modes[m]);
  VCMatrix matrix={0};matrix.route[0]=(VCMatrixRoute){3,58,amount};vc_set_matrix(&s,&matrix);vc_note_on(&s,60,100);
  VCVoice *v=&s.voice[0];s.controllers[3]=src[t];v->lfo3.value=lfo[t];s.global_lfo[2].value=lfo[t];
  v->osc[0].phase=.31f;v->osc[1].phase=.29f;v->osc3.phase=.17f;v->osc[0].pending=.007f;
  vc_filter_control(&s,v);
  assert(v->osc[0].phase==.31f&&v->osc[1].phase==.29f&&v->osc3.phase==.17f&&v->osc[0].pending==.007f);
  printf("%u %u %u %u %u %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",dest,amount,bases[b],t,modes[m],v->pitch1_base_inc,v->pitch2_base_inc,v->osc3_base_inc,v->inc[0],v->inc[1],v->inc3,v->pw_last[0],v->pw_last[1]);
 }
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_LFO3_ENABLE,1);vc_param(&s,VC_LFO3_AMOUNT,64);vc_param(&s,VC_LFO3_FADE,0);vc_note_on(&s,60,100);
 VCMatrix matrix={0};matrix.route[0]=(VCMatrixRoute){3,58,127};vc_set_matrix(&s,&matrix);s.controllers[3]=1;s.voice[0].lfo3.value=.5f;vc_filter_control(&s,&s.voice[0]);
 vc_set_matrix(&s,0);s.voice[0].lfo3.value=.5f;vc_filter_control(&s,&s.voice[0]);assert(fabsf(s.voice[0].pitch1_last-vc_lfo3Pitch[64]*.5f)<.00001f);
 vc_set_matrix(&s,&matrix);VCSynth b=s;float l[256],r[256],ll[256],rr[256];vc_render_stereo(&s,l,r,256);for(unsigned j=0;j<256;j++)vc_render_stereo(&b,ll+j,rr+j,1);
 assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));for(unsigned j=0;j<256;j++)assert(__builtin_isfinite(l[j])&&__builtin_isfinite(r[j]));
}
