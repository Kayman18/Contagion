#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const float sources[]={-1,0,.37f,1};const unsigned bases[]={0,40,64,100,127};
 for(unsigned curve=0;curve<15;curve++)for(unsigned b=0;b<5;b++)for(unsigned a=0;a<128;a++)for(unsigned t=0;t<4;t++){
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_SAT_CURVE,curve);vc_param(&s,VC_SAT_DRIVE,bases[b]);vc_param(&s,VC_SUB_VOLUME,80);vc_param(&s,VC_OSC3_MODE,2);vc_param(&s,VC_OSC3_VOLUME,100);vc_param(&s,VC_PUNCH,50);vc_note_on(&s,72,100);VCVoice *v=&s.voice[0];VCMatrix m={0};m.route[0]=(VCMatrixRoute){3,22,a};vc_set_matrix(&s,&m);s.controllers[3]=sources[t];v->saturation.lp=.125f;v->saturation.hold=.25f;v->saturation.counter=.75f;vc_filter_control(&s,v);assert(v->saturation.lp==.125f&&v->saturation.hold==.25f&&v->saturation.counter==.75f);
 printf("%u %u %u %u %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",curve,bases[b],a,t,v->source_gain[5],v->source_gain[0],v->osc3_gain,v->punch_gain,v->saturation.level,v->saturation.period,v->saturation.coef);
 vc_set_matrix(&s,0);assert(v->saturation.level==(bases[b]<64?64:bases[b]));
 }
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_SAT_DRIVE,20);VCMatrix m={0};m.route[0]=(VCMatrixRoute){24,22,127};vc_set_matrix(&s,&m);vc_note_on(&s,60,20);vc_note_on(&s,64,110);for(unsigned i=0;i<2;i++)vc_filter_control(&s,&s.voice[i]);assert(s.voice[0].source_gain[5]!=s.voice[1].source_gain[5]);
 VCSynth b=s;float l[256],r[256],ll[256],rr[256];vc_render_stereo(&s,l,r,256);for(unsigned i=0;i<256;i++)vc_render_stereo(&b,ll+i,rr+i,1);assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));
}
