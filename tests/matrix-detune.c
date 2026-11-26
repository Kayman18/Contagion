#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const float levels[]={-1,0,.37f,1};const unsigned bases[]={0,32,64,127};
 for(unsigned b=0;b<4;b++)for(unsigned a=0;a<128;a++)for(unsigned t=0;t<4;t++)for(unsigned mode=1;mode<=4;mode++){
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_DETUNE,bases[b]);vc_param(&s,VC_OSC3_MODE,mode);VCMatrix m={0};m.route[0]=(VCMatrixRoute){3,15,a};vc_set_matrix(&s,&m);vc_note_on(&s,60,100);VCVoice *v=&s.voice[0];s.controllers[3]=levels[t];v->osc[1].phase=.17f;v->osc3.phase=.19f;v->osc[1].pending=.003f;v->osc3.pending=.004f;vc_filter_control(&s,v);
 assert(v->osc[1].phase==.17f&&v->osc3.phase==.19f&&v->osc[1].pending==.003f&&v->osc3.pending==.004f);assert(v->osc[1].band==vc_osc_band(v->inc[1])&&v->osc3.band==vc_osc_band(v->inc3));
 printf("%u %u %u %u %.9g %.9g %.9g %.9g %.9g\n",bases[b],a,t,mode,v->osc2_matrix_detune,v->pitch2_base_inc,v->osc3_base_inc,v->inc[1],v->inc3);
 vc_set_matrix(&s,0);vc_filter_control(&s,v);assert(v->osc2_matrix_detune==0&&v->inc[1]==v->pitch2_base_inc&&v->inc3==v->osc3_base_inc);
 }
 VCSynth s;vc_init(&s,44100);VCMatrix m={0};m.route[0]=(VCMatrixRoute){24,15,127};vc_set_matrix(&s,&m);vc_note_on(&s,60,20);vc_note_on(&s,60,110);for(unsigned i=0;i<2;i++)vc_filter_control(&s,&s.voice[i]);assert(s.voice[0].osc2_matrix_detune!=s.voice[1].osc2_matrix_detune);
 VCSynth b=s;float l[256],r[256],ll[256],rr[256];vc_render_stereo(&s,l,r,256);for(unsigned i=0;i<256;i++)vc_render_stereo(&b,ll+i,rr+i,1);assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));
}
