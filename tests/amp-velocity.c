#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 const unsigned amounts[]={0,32,64,96,127,128},velocities[]={1,32,64,100,127};
 for(unsigned amount=0;amount<=128;amount++)for(unsigned velocity=0;velocity<=127;velocity++)emit(vc_amp_velocity(amount,velocity/127.f));
 for(unsigned route=0;route<4;route++)for(unsigned amount=0;amount<6;amount++)for(unsigned vel=0;vel<5;vel++){
  VCSynth s;vc_init(&s,44100);s.param[VC_ROUTING]=route;s.param[VC_OSC3_MODE]=2;s.param[VC_AMP_VELOCITY]=amounts[amount];s.param[VC_SHAPE2]=32;s.param[VC_PW2]=80;s.param[VC_CUTOFF]=73;s.param[VC_CUTOFF2]=91;s.param[VC_RESONANCE]=55;s.param[VC_RESONANCE2]=70;vc_route_setup(&s);vc_note_on(&s,60,velocities[vel]);VCVoice *v=&s.voice[0];vc_filter_control(&s,v);
  v->inc[0]=1.f/128;v->inc[1]=1.f/64;v->inc3=1.f/256;v->control_remaining=10000;v->env.level=v->env.sustain=1;v->env.stage=3;v->filter_env.level=0;
  vc_osc_setup(&v->osc[0],0,64,.5f,v->inc[0]);vc_osc_setup(&v->osc[1],0,32,.5f*(1-80.f/127),v->inc[1]);v->osc3.band=vc_osc_band(v->inc3);v->osc3.table=v->osc3.tables[v->osc3.band];
  for(unsigned i=0;i<256;i++){float l,r;vc_render_stereo(&s,&l,&r,1);emit(l);emit(r);}
 }

 for(unsigned amount=0;amount<=128;amount++)for(unsigned vel=0;vel<5;vel++){
  VCSynth s;vc_init(&s,44100);vc_note_on(&s,60,velocities[vel]);VCVoice *v=&s.voice[0];vc_param(&s,VC_AMP_VELOCITY,amount);
  for(unsigned i=0;i<40;i++){emit(vc_amp_gain_next(v));}
  assert(v->amp_remaining==0);
  vc_param(&s,VC_AMP_VELOCITY,128);for(unsigned i=0;i<32;i++)vc_amp_gain_next(v);assert(v->amp_gain==v->velocity);
 }
}
