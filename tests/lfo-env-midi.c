#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
int main(void){
 const int cc[2]={69,81};const unsigned id[2]={VC_LFO_MODE,VC_LFO2_MODE};
 unsigned checks=0;
 for(unsigned k=0;k<2;k++)for(unsigned old=0;old<3;old++)for(int value=-1;value<=128;value++){
  VCSynth a,b;vc_init(&a,44100);vc_param(&a,id[k],old);vc_param(&a,VC_LFO_OSC1,90);vc_param(&a,VC_LFO2_CUTOFF1,90);vc_param(&a,k?VC_LFO2_SHAPE:VC_LFO_SHAPE,67);vc_note_on(&a,60,100);b=a;
  VCLfo *l=k?&a.voice[0].lfo2:&a.voice[0].lfo;float phase=l->phase;
  assert(vc_cc_param(cc[k])==(int)id[k]);vc_cc(&a,cc[k],value);vc_param(&b,id[k],value>0?2:1);
  assert(a.param[id[k]]==(value>0?2:1));assert(l->phase==phase);assert(l->env_mode==(value>0));
  assert(!memcmp(&a,&b,sizeof a));float x[96],y[96],xx[96],yy[96];vc_render_stereo(&a,x,y,96);vc_render_stereo(&b,xx,yy,96);assert(!memcmp(x,xx,sizeof x)&&!memcmp(y,yy,sizeof y));
  if(value>=0&&value<=127)printf("%u %d %d %u\n",k,value,old,a.param[id[k]]);
  checks++;
 }
 for(unsigned k=0;k<2;k++){
  VCSynth a;vc_init(&a,44100);vc_cc(&a,cc[k],1);vc_note_on(&a,60,100);
  VCLfo *l=k?&a.voice[0].lfo2:&a.voice[0].lfo;vc_lfo_tick(l,441000);assert(l->done);
  float phase=l->phase;vc_cc(&a,cc[k],0);assert(!l->done&&!l->env_mode&&l->phase==phase);
 }
 assert(checks==780);assert(vc_cc_param(70)==VC_LFO_GLOBAL&&vc_cc_param(82)==VC_LFO2_GLOBAL);
 return 0;
}
