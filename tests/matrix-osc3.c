#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const unsigned bases[]={0,64,127};const float sources[]={-1,0,.37f,1};
 for(unsigned dest=97;dest<=99;dest++)for(unsigned a=0;a<128;a++)for(unsigned b=0;b<3;b++)for(unsigned t=0;t<4;t++)for(unsigned mode=0;mode<=4;mode++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_OSC3_MODE,mode);vc_param(&s,VC_OSC3_VOLUME,bases[b]);vc_param(&s,VC_OSC3_TUNE,bases[b]);vc_param(&s,VC_OSC3_DETUNE,bases[b]);
  VCMatrix matrix={0};matrix.route[0]=(VCMatrixRoute){3,dest,a};vc_set_matrix(&s,&matrix);vc_note_on(&s,60,100);VCVoice *v=&s.voice[0];
  s.controllers[3]=sources[t];v->osc3.phase=.17f;v->osc3.pending=.003f;vc_filter_control(&s,v);
  assert(v->osc3.phase==.17f&&v->osc3.pending==.003f);if(mode)assert(v->osc3.band==vc_osc_band(v->inc3));
  printf("%u %u %u %u %u %.9g %.9g %.9g\n",dest,a,bases[b],t,mode,v->osc3_base_inc,v->inc3,v->osc3_gain);
 }
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_OSC3_MODE,4);vc_note_on(&s,60,20);vc_note_on(&s,64,110);
 float base=s.voice[0].inc3,gain=s.voice[0].osc3_gain;VCMatrix m={0};m.route[0]=(VCMatrixRoute){24,98,127};m.route[1]=(VCMatrixRoute){24,97,0};vc_set_matrix(&s,&m);
 for(unsigned i=0;i<2;i++)vc_filter_control(&s,&s.voice[i]);
 assert(s.voice[0].osc3_matrix_cents!=s.voice[1].osc3_matrix_cents&&s.voice[0].osc3_gain!=s.voice[1].osc3_gain);
 vc_set_matrix(&s,0);vc_filter_control(&s,&s.voice[0]);assert(s.voice[0].inc3==base&&s.voice[0].osc3_gain==gain);
 vc_set_matrix(&s,&m);VCSynth b=s;float l[256],r[256],ll[256],rr[256];vc_render_stereo(&s,l,r,256);
 for(unsigned j=0;j<256;j++)vc_render_stereo(&b,ll+j,rr+j,1);
 assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));
}
