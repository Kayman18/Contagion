#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const float source[]={-1,0,.37f,1};const unsigned follow[]={0,64,96,127};
 for(unsigned k=0;k<4;k++)for(unsigned a=0;a<128;a++)for(unsigned t=0;t<4;t++)for(unsigned mode=1;mode<=4;mode++){
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_KEYFOLLOW1,follow[k]);vc_param(&s,VC_KEYFOLLOW2,follow[3-k]);vc_param(&s,VC_OSC3_MODE,mode);vc_param(&s,VC_SAT_CURVE,12);vc_note_on(&s,72,100);VCVoice *v=&s.voice[0];VCMatrix m={0};m.route[0]=(VCMatrixRoute){3,4,a};vc_set_matrix(&s,&m);s.controllers[3]=source[t];v->osc[0].phase=.17f;v->osc[1].phase=.19f;v->osc3.phase=.21f;vc_filter_control(&s,v);assert(v->osc[0].phase==.17f&&v->osc[1].phase==.19f&&v->osc3.phase==.21f);
 printf("%u %u %u %u %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",k,a,t,mode,v->transpose_matrix,v->pitch1_base_inc,v->pitch2_base_inc,v->osc3_base_inc,v->inc[0],v->inc[1],v->inc3,v->saturation.semitones);
 vc_set_matrix(&s,0);vc_filter_control(&s,v);assert(v->transpose_matrix==0&&v->saturation.semitones==12);
 }
 VCSynth s;vc_init(&s,44100);VCMatrix m={0};m.route[0]=(VCMatrixRoute){24,4,70};vc_set_matrix(&s,&m);vc_note_on(&s,60,20);vc_note_on(&s,60,110);for(unsigned i=0;i<2;i++)vc_filter_control(&s,&s.voice[i]);assert(s.voice[0].transpose_matrix!=s.voice[1].transpose_matrix);
 VCSynth b=s;float l[256],r[256],ll[256],rr[256];vc_render_stereo(&s,l,r,256);for(unsigned i=0;i<256;i++)vc_render_stereo(&b,ll+i,rr+i,1);assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));
 /* Equivalent base-transpose and matrix-transpose patches track filters alike. */
 VCSynth x,y;vc_init(&x,44100);vc_init(&y,44100);vc_param(&x,VC_TRANSPOSE,76);vc_param(&x,VC_F_KEYFOLLOW1,100);vc_param(&y,VC_F_KEYFOLLOW1,100);vc_param(&x,VC_F_KEYFOLLOW2,24);vc_param(&y,VC_F_KEYFOLLOW2,24);VCMatrix mm={0};mm.route[0]=(VCMatrixRoute){3,4,70};vc_set_matrix(&y,&mm);y.controllers[3]=1;vc_note_on(&x,72,100);vc_note_on(&y,72,100);vc_filter_control(&x,&x.voice[0]);vc_filter_control(&y,&y.voice[0]);for(unsigned i=0;i<2;i++)assert(fabsf(x.voice[0].filter[i].g-y.voice[0].filter[i].g)<1e-6f);
}
