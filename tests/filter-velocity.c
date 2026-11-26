#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 for(unsigned i=0;i<=2048;i++)emit(vc_velocity_env_amount(i/16.f));
 const unsigned velocities[]={1,32,64,100,127};const float envs[]={0,.5f,1};
 for(unsigned route=0;route<4;route++)for(unsigned amount=0;amount<128;amount++)for(unsigned vel=0;vel<5;vel++)for(unsigned env=0;env<3;env++){
  VCSynth s;vc_init(&s,44100);s.param[VC_ROUTING]=route;s.param[VC_FILTER_MODE]=route;s.param[VC_FILTER_MODE2]=(route+1)%4;
  s.param[VC_CUTOFF]=32;s.param[VC_CUTOFF2]=96;s.param[VC_RESONANCE]=55;s.param[VC_RESONANCE2]=70;
  s.param[VC_F_AMOUNT1]=96;s.param[VC_F_AMOUNT2]=24;s.param[VC_F_POLARITY2]=0;
  s.param[VC_VEL_ENV1]=s.param[VC_VEL_RES1]=amount;s.param[VC_VEL_ENV2]=s.param[VC_VEL_RES2]=127-amount;
  s.param[VC_LFO_MODE]=1;s.param[VC_LFO_RESONANCE]=80;vc_route_setup(&s);vc_note_on(&s,60,velocities[vel]);VCVoice *v=&s.voice[0];v->filter_env.level=envs[env];v->lfo.value=.25f;vc_filter_control(&s,v);
  emit(v->filter[0].cutoff);emit(v->filter[1].cutoff);emit(v->filter[0].resonance);emit(v->filter[1].resonance);
  for(unsigned n=0;n<64;n++){float l,r;vc_route_sample(&s,v,(n%31-15.f)/128,(n%23-11.f)/128,&l,&r);emit(l);emit(r);}
 }
}
