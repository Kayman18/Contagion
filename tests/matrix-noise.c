#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const float source[]={-1,0,.37f,1};
 for(unsigned sr=44100;sr<=48000;sr+=3900)for(unsigned b=0;b<128;b++)for(unsigned a=0;a<128;a++)for(unsigned t=0;t<4;t++){
 VCSynth s;vc_init(&s,sr);vc_param(&s,VC_NOISE_COLOR,b);VCMatrix m={0};m.route[0]=(VCMatrixRoute){3,91,a};vc_set_matrix(&s,&m);vc_note_on(&s,60,100);VCVoice *v=&s.voice[0];s.controllers[3]=source[t];v->noise.lp=.125f;uint32_t rng=v->noise.rng;vc_filter_control(&s,v);assert(v->noise.lp==.125f&&v->noise.rng==rng);
 printf("%u %u %u %u %d %.9g %.9g\n",sr,b,a,t,v->noise.color,v->noise.gain,v->noise.coef);
 vc_set_matrix(&s,0);assert(v->noise.color==(int)b&&v->noise.lp==.125f&&v->noise.rng==rng);
 }
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_NOISE_VOLUME,100);VCMatrix m={0};m.route[0]=(VCMatrixRoute){24,91,100};vc_set_matrix(&s,&m);vc_note_on(&s,60,20);vc_note_on(&s,64,110);for(unsigned i=0;i<2;i++)vc_filter_control(&s,&s.voice[i]);assert(s.voice[0].noise.color!=s.voice[1].noise.color);
 VCSynth b=s;float l[256],r[256],ll[256],rr[256];vc_render_stereo(&s,l,r,256);for(unsigned i=0;i<256;i++)vc_render_stereo(&b,ll+i,rr+i,1);assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));
}
