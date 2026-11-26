#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 for(unsigned i=0;i<128;i++)emit(vc_punchLevel[i]);
 const unsigned levels[]={0,1,32,64,96,127},volumes[]={0,32,64,96,127},curves[]={0,2,9,11,13};
 for(unsigned level=0;level<6;level++)for(unsigned vol=0;vol<5;vol++)for(unsigned route=0;route<4;route++)for(unsigned curve=0;curve<5;curve++){
  VCSynth s;vc_init(&s,44100);s.param[VC_PUNCH]=levels[level];s.param[VC_SAT_DRIVE]=volumes[vol];s.param[VC_ROUTING]=route;s.param[VC_SAT_CURVE]=curves[curve];s.param[VC_CUTOFF]=73;s.param[VC_CUTOFF2]=91;s.param[VC_RESONANCE]=55;s.param[VC_RESONANCE2]=70;vc_route_setup(&s);vc_note_on(&s,60,127);VCVoice *v=&s.voice[0];vc_filter_control(&s,v);
  if(curves[curve]==9)v->saturation.period=floorf(v->saturation.period*16+.5f)/16;
  for(unsigned i=0;i<256;i++){float l,r;vc_route_sample(&s,v,0,0,&l,&r);emit(l);emit(r);}
  assert(v->punch_at==12);
 }
 /* No control change, bend or release retriggers the impulse; a new voice does. */
 VCSynth s;vc_init(&s,44100);vc_note_on(&s,60,127);VCVoice *v=&s.voice[0];float l,r;
 for(unsigned i=0;i<20;i++)vc_route_sample(&s,v,0,0,&l,&r);
 assert(v->punch_at==12);vc_param(&s,VC_PUNCH,127);vc_bend(&s,10000);assert(v->punch_at==12&&v->punch_gain>0);
 vc_note_off(&s,60);assert(v->punch_at==12);vc_panic(&s);vc_note_on(&s,60,127);assert(s.voice[0].punch_at==0);
 vc_param(&s,VC_SAT_DRIVE,0);assert(s.voice[0].punch_gain==vc_punchLevel[127]*vc_oscVolume[0]);
 vc_param(&s,VC_PUNCH,0);assert(s.voice[0].punch_gain==0);
}
