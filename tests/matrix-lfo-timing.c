#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 const unsigned dests[]={43,44,50,51,57},bases[]={0,64,126,127},clocks[]={0,1,21};
 const unsigned rate_ids[]={VC_LFO_RATE,VC_LFO2_RATE,VC_LFO3_RATE},key_ids[]={VC_LFO_KEYTRACK,VC_LFO2_KEYTRACK,VC_LFO3_KEYTRACK},clock_ids[]={VC_LFO_CLOCK,VC_LFO2_CLOCK,VC_LFO3_CLOCK},scope_ids[]={VC_LFO_GLOBAL,VC_LFO2_GLOBAL,VC_LFO3_GLOBAL};
 const float levels[]={-1,0,.37f,1};
 for(unsigned d=0;d<5;d++)for(unsigned a=0;a<128;a++)for(unsigned b=0;b<4;b++)for(unsigned t=0;t<4;t++)for(unsigned c=0;c<3;c++){
  VCSynth s;vc_init(&s,44100);unsigned index=d/2,clock=clocks[c],id=d==1?VC_LFO_CONTOUR:d==3?VC_LFO2_CONTOUR:rate_ids[index];
  vc_param(&s,id,bases[b]);vc_param(&s,key_ids[index],64);vc_param(&s,clock_ids[index],clock);vc_param(&s,scope_ids[index],t&1);
  vc_param(&s,VC_LFO_MODE,1);vc_param(&s,VC_LFO2_MODE,1);vc_param(&s,VC_LFO3_ENABLE,1);
  VCMatrix matrix={0};matrix.route[0]=(VCMatrixRoute){3,dests[d],a};vc_set_matrix(&s,&matrix);unsigned note=t<2?48:84;vc_note_on(&s,note,100);
  VCVoice *v=&s.voice[0];VCLfo *l=index==0?&v->lfo:index==1?&v->lfo2:&v->lfo3;
  float dev=index==0?v->lfo_deviation:index==1?v->lfo2_deviation:v->lfo3_deviation;
  float global_rate=s.global_lfo[index].rate,global_phase=s.global_lfo[index].phase,global_contour=s.global_lfo[index].contour;
  s.controllers[3]=levels[t];l->phase=.25f;l->cycles=.125f;l->value=.2f;l->start=.125f;
  vc_filter_control(&s,v);
  assert(s.global_lfo[index].rate==global_rate&&s.global_lfo[index].phase==global_phase&&s.global_lfo[index].contour==global_contour);
  assert(l->start==.125f);
  printf("%u %u %u %u %u %u %.9g %.9g %.9g %.9g %.9g %.9g\n",dests[d],a,bases[b],t,clock,note,dev,l->rate,l->contour,l->phase,l->cycles,l->value);
 }
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_LFO_MODE,2);vc_note_on(&s,60,100);float rate=s.voice[0].lfo.rate;
 VCMatrix matrix={0};matrix.route[0]=(VCMatrixRoute){3,43,127};vc_set_matrix(&s,&matrix);s.controllers[3]=1;s.voice[0].lfo.done=1;s.voice[0].lfo.phase=.125f;
 vc_filter_control(&s,&s.voice[0]);assert(s.voice[0].lfo.phase==.125f&&s.voice[0].lfo.done);
 vc_set_matrix(&s,0);assert(s.voice[0].lfo.rate==rate&&s.voice[0].lfo.phase==.125f&&s.voice[0].lfo.done);
 vc_param(&s,VC_LFO_MODE,1);vc_set_matrix(&s,&matrix);VCSynth b=s;float l[256],r[256],ll[256],rr[256];
 vc_render_stereo(&s,l,r,256);for(unsigned j=0;j<256;j++)vc_render_stereo(&b,ll+j,rr+j,1);
 assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));
}
