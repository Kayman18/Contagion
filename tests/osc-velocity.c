#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 const unsigned velocities[]={1,32,64,100,127};const float values[]={-.25f,.5f};
 for(unsigned route=0;route<4;route++)for(unsigned mode=1;mode<=4;mode++)for(unsigned amount=0;amount<128;amount++)for(unsigned vel=0;vel<5;vel++){
  VCSynth s;vc_init(&s,44100);s.param[VC_ROUTING]=route;s.param[VC_OSC3_MODE]=mode;s.param[VC_SHAPE1]=16;s.param[VC_SHAPE2]=96;s.param[VC_PW1]=40;s.param[VC_PW2]=80;
  s.param[VC_CUTOFF]=73;s.param[VC_CUTOFF2]=91;s.param[VC_RESONANCE]=55;s.param[VC_RESONANCE2]=70;
  s.param[VC_VEL_SHAPE1]=s.param[VC_VEL_PW]=amount;s.param[VC_VEL_SHAPE2]=s.param[VC_VEL_PAN]=127-amount;
  s.param[VC_LFO_MODE]=s.param[VC_LFO2_MODE]=1;s.param[VC_LFO_PW]=88;s.param[VC_LFO2_SHAPE_AMOUNT]=96;s.param[VC_LFO2_PAN]=96;
  vc_route_setup(&s);vc_note_on(&s,60,velocities[vel]);VCVoice *v=&s.voice[0];
  for(unsigned step=0;step<2;step++){
   v->lfo.value=v->lfo2.value=values[step];vc_filter_control(&s,v);
   for(unsigned i=0;i<3;i++){VCOsc *o=i==2?&v->osc3:&v->osc[i];emit(o->wave_gain);emit(o->saw_gain);emit(o->pulse_gain);emit(o->duty);}
   for(unsigned n=0;n<32;n++){unsigned k=step*32+n;float l,r;vc_route_sample(&s,v,(k%31-15.f)/128,(k%23-11.f)/128,&l,&r);emit(l);emit(r);for(unsigned i=0;i<4;i++)emit(v->pan_now[i]);}
  }
  vc_param(&s,VC_VEL_PAN,64);vc_param(&s,VC_LFO2_PAN,64);vc_filter_control(&s,v);for(unsigned n=0;n<32;n++){float l,r;vc_route_sample(&s,v,0,0,&l,&r);}assert(!v->pan_active);for(unsigned i=0;i<4;i++)assert(v->pan_now[i]==s.pan[i]);
 }
}
