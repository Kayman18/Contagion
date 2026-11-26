#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const float levels[]={-1,0,.37f,1};const unsigned bases[]={0,64,96,127},notes[]={36,60,84};
 for(unsigned d=0;d<2;d++)for(unsigned b=0;b<4;b++)for(unsigned a=0;a<128;a++)for(unsigned t=0;t<4;t++)for(unsigned mode=1;mode<=4;mode++)for(unsigned n=0;n<3;n++){
 VCSynth s;vc_init(&s,44100);vc_param(&s,d?VC_KEYFOLLOW2:VC_KEYFOLLOW1,bases[b]);vc_param(&s,VC_OSC3_MODE,mode);VCMatrix m={0};m.route[0]=(VCMatrixRoute){3,d?19:10,a};vc_set_matrix(&s,&m);vc_note_on(&s,notes[n],100);VCVoice *v=&s.voice[0];s.controllers[3]=levels[t];v->osc[0].phase=.11f;v->osc[1].phase=.17f;v->osc3.phase=.19f;v->osc[0].pending=.002f;v->osc[1].pending=.003f;v->osc3.pending=.004f;vc_filter_control(&s,v);
 assert(v->osc[0].phase==.11f&&v->osc[1].phase==.17f&&v->osc3.phase==.19f&&v->osc[0].pending==.002f&&v->osc[1].pending==.003f&&v->osc3.pending==.004f);assert(v->osc[0].band==vc_osc_band(v->inc[0])&&v->osc[1].band==vc_osc_band(v->inc[1])&&v->osc3.band==vc_osc_band(v->inc3));
 printf("%u %u %u %u %u %u %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",d,bases[b],a,t,mode,notes[n],v->key_matrix_cents[d],v->pitch1_base_inc,v->pitch2_base_inc,v->osc3_base_inc,v->inc[0],v->inc[1],v->inc3);
 vc_set_matrix(&s,0);vc_filter_control(&s,v);assert(v->key_matrix_cents[0]==0&&v->key_matrix_cents[1]==0);
 }
 for(unsigned layers=1;layers<=4;layers++){
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_UNISON_MODE,layers-1);vc_param(&s,VC_UNISON_DETUNE,64);vc_param(&s,VC_PORTAMENTO,90);vc_param(&s,VC_OSC3_MODE,4);VCMatrix m={0};m.route[0]=(VCMatrixRoute){3,10,72};m.route[1]=(VCMatrixRoute){3,19,72};vc_set_matrix(&s,&m);vc_note_on(&s,72,100);s.controllers[3]=.5f;s.pitch_offset=2.25f;
 for(unsigned frame=0;frame<8;frame++)for(unsigned i=0;i<layers;i++){
 VCVoice *v=&s.voice[i];v->pitch=63.125f+frame*.25f;v->glide_error=0;vc_pitch_bases(&s,v);vc_filter_control(&s,v);float expected=(v->pitch+s.pitch_offset+v->unison_cents/100-60)*25;
 assert(fabsf(v->key_matrix_cents[0]-expected)<.001f&&fabsf(v->key_matrix_cents[1]-expected)<.001f);
 }
 }
 VCSynth s;vc_init(&s,44100);VCMatrix m={0};m.route[0]=(VCMatrixRoute){24,19,127};vc_set_matrix(&s,&m);vc_note_on(&s,72,20);vc_note_on(&s,72,60);for(unsigned i=0;i<2;i++)vc_filter_control(&s,&s.voice[i]);assert(s.voice[0].key_matrix_cents[1]!=s.voice[1].key_matrix_cents[1]);
 VCSynth b=s;float l[256],r[256],ll[256],rr[256];vc_render_stereo(&s,l,r,256);for(unsigned i=0;i<256;i++)vc_render_stereo(&b,ll+i,rr+i,1);assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));
}
