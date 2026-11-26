#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 const unsigned amounts[]={0,32,64,96,127},pans[]={0,64,127},spreads[]={0,64,127};const float values[]={-1,-.25f,.5f,1};
 for(unsigned route=0;route<4;route++)for(unsigned mode=0;mode<4;mode++)for(unsigned a=0;a<5;a++)for(unsigned p=0;p<3;p++)for(unsigned sp=0;sp<3;sp++){
  VCSynth s;vc_init(&s,44100);s.param[VC_ROUTING]=route;s.param[VC_FILTER_BALANCE]=64;s.param[VC_PAN]=pans[p];s.param[VC_SPREAD]=spreads[sp];s.param[VC_LFO2_PAN]=amounts[a];s.param[VC_LFO2_MODE]=1;vc_route_setup(&s);
  VCVoice *v=&s.voice[0];vc_filter_setup(&v->filter[0],44100,mode,route==1?4:2,73,55);vc_filter_setup(&v->filter[1],44100,(mode+1)%4,2,91,70);
  for(unsigned n=0;n<128;n++){
   if(!(n%32)){v->lfo2_mod=values[n/32];vc_lfo2_pan_control(&s,v);}
   float l,r;vc_route_sample(&s,v,(n%31-15.f)/128,(n%23-11.f)/128,&l,&r);emit(l);emit(r);
   for(unsigned i=0;i<4;i++)emit(v->pan_now[i]);
  }
  s.param[VC_LFO2_MODE]=0;v->lfo2_mod=0;vc_lfo2_pan_control(&s,v);
  for(unsigned n=0;n<32;n++){float l,r;vc_route_sample(&s,v,0,0,&l,&r);}
  assert(!v->pan_active);for(unsigned i=0;i<4;i++)assert(v->pan_now[i]==s.pan[i]);
 }
 for(unsigned route=0;route<4;route+=3)for(unsigned shape=0;shape<4;shape++){
  VCSynth s;vc_init(&s,44100);s.param[VC_ROUTING]=route;s.param[VC_LFO2_PAN]=96;s.param[VC_LFO2_MODE]=1;s.param[VC_LFO2_RATE]=96;s.param[VC_LFO2_SHAPE]=shape;vc_route_setup(&s);vc_note_on(&s,60,100);
  VCVoice *v=&s.voice[0];v->lfo2_deviation=1;vc_update_voice(&s,v);
  for(unsigned n=0;n<12000;n++){float l,r;vc_render_stereo(&s,&l,&r,1);for(unsigned i=0;i<4;i++)emit(v->pan_now[i]);}
 }
}
