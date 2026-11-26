#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const float levels[]={-1,0,.37f,1};const unsigned bases[]={0,64,127},dest[]={21,23,90},ids[]={VC_SUB_VOLUME,VC_NOISE_VOLUME,VC_RING_VOLUME};
 for(unsigned d=0;d<3;d++)for(unsigned b=0;b<3;b++)for(unsigned a=0;a<128;a++)for(unsigned t=0;t<4;t++){
 VCSynth s;vc_init(&s,44100);vc_param(&s,ids[d],bases[b]);vc_param(&s,VC_SAT_DRIVE,40);VCMatrix m={0};m.route[0]=(VCMatrixRoute){3,dest[d],a};vc_set_matrix(&s,&m);vc_note_on(&s,60,100);VCVoice *v=&s.voice[0];s.controllers[3]=levels[t];v->sub.phase=.17f;v->noise.lp=.125f;uint32_t rng=v->noise.rng;vc_filter_control(&s,v);assert(v->sub.phase==.17f&&v->noise.lp==.125f&&v->noise.rng==rng);
 printf("%u %u %u %u %.9g\n",d,bases[b],a,t,v->source_gain[d]);
 float expected=vc_sourceGain[bases[b]]*(d==0?vc_oscVolume[40]:d==2?1.5f:1);vc_set_matrix(&s,0);assert(fabsf(v->source_gain[d]-expected)<1e-6f);
 }
 /* Route can enable a source whose base level is zero, then mute it again. */
 for(unsigned d=0;d<3;d++){
 VCSynth s;vc_init(&s,44100);for(unsigned i=0;i<3;i++)vc_param(&s,ids[i],0);VCMatrix m={0};m.route[0]=(VCMatrixRoute){3,dest[d],127};vc_set_matrix(&s,&m);vc_note_on(&s,60,100);s.controllers[3]=1;VCVoice *v=&s.voice[0];vc_filter_control(&s,v);float l,r,peak=0;for(unsigned j=0;j<32;j++){vc_mix_sources(&s,v,.3f,.4f,0,0,&l,&r);peak+=fabsf(l)+fabsf(r);}assert(peak>0);s.controllers[3]=0;vc_filter_control(&s,v);vc_mix_sources(&s,v,.3f,.4f,0,0,&l,&r);assert(l==0&&r==0);
 }
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_SUB_VOLUME,0);VCMatrix m={0};m.route[0]=(VCMatrixRoute){24,21,127};vc_set_matrix(&s,&m);vc_note_on(&s,60,20);vc_note_on(&s,64,110);for(unsigned i=0;i<2;i++)vc_filter_control(&s,&s.voice[i]);assert(s.voice[0].source_gain[0]!=s.voice[1].source_gain[0]);
 VCSynth b=s;float l[256],r[256],ll[256],rr[256];vc_render_stereo(&s,l,r,256);for(unsigned i=0;i<256;i++)vc_render_stereo(&b,ll+i,rr+i,1);assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));
}
