#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const float levels[]={-1,0,.37f,1};const unsigned bases[]={0,64,126,127};
 for(unsigned b=0;b<4;b++)for(unsigned a=0;a<128;a++)for(unsigned t=0;t<4;t++)for(unsigned mode=0;mode<=4;mode++){
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_BALANCE,bases[b]);vc_param(&s,VC_OSC3_MODE,mode);vc_param(&s,VC_SAT_DRIVE,40);vc_note_on(&s,60,100);VCVoice *v=&s.voice[0];float base3=v->osc3_gain;VCMatrix m={0};m.route[0]=(VCMatrixRoute){3,20,a};vc_set_matrix(&s,&m);s.controllers[3]=levels[t];v->osc[0].phase=.17f;v->osc[1].phase=.19f;v->osc3.phase=.21f;vc_filter_control(&s,v);assert(v->osc[0].phase==.17f&&v->osc[1].phase==.19f&&v->osc3.phase==.21f);
 printf("%u %u %u %u %.9g %.9g %.9g %.9g\n",bases[b],a,t,mode,v->source_gain[3],v->source_gain[4],v->osc3_gain,base3);
 vc_set_matrix(&s,0);vc_filter_control(&s,v);assert(fabsf(v->osc3_gain-base3)<1e-6f);
 }
 VCSynth s;vc_init(&s,44100);VCMatrix m={0};m.route[0]=(VCMatrixRoute){24,20,100};vc_set_matrix(&s,&m);vc_note_on(&s,60,20);vc_note_on(&s,64,110);for(unsigned i=0;i<2;i++)vc_filter_control(&s,&s.voice[i]);assert(s.voice[0].source_gain[3]!=s.voice[1].source_gain[3]);
 VCSynth b=s;float l[256],r[256],ll[256],rr[256];vc_render_stereo(&s,l,r,256);for(unsigned i=0;i<256;i++)vc_render_stereo(&b,ll+i,rr+i,1);assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));
}
