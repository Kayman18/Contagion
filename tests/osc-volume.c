#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
for(unsigned i=0;i<128;i++)emit(vc_oscVolume[i]);
 for(unsigned volume=0;volume<128;volume++)for(unsigned route=0;route<4;route++)for(unsigned mode=1;mode<=4;mode++){
  VCSynth s;vc_init(&s,44100);s.param[VC_SAT_DRIVE]=volume;s.param[VC_SAT_CURVE]=2;s.param[VC_SUB_VOLUME]=37;s.param[VC_RING_VOLUME]=39;s.param[VC_ROUTING]=route;s.param[VC_OSC3_MODE]=mode;s.param[VC_SHAPE2]=32;s.param[VC_PW2]=80;s.param[VC_CUTOFF]=73;s.param[VC_CUTOFF2]=91;s.param[VC_RESONANCE]=55;s.param[VC_RESONANCE2]=70;vc_route_setup(&s);vc_note_on(&s,60,127);VCVoice *v=&s.voice[0];vc_filter_control(&s,v);
  v->inc[0]=1.f/128;v->inc[1]=1.f/64;v->inc3=1.f/256;v->control_remaining=10000;v->env.level=v->env.sustain=1;v->env.stage=3;v->filter_env.level=0;
  vc_osc_setup(&v->osc[0],0,64,.5f,v->inc[0]);vc_osc_setup(&v->osc[1],0,32,.5f*(1-80.f/127),v->inc[1]);v->osc3.band=vc_osc_band(v->inc3);v->osc3.table=v->osc3.tables[v->osc3.band];
  for(unsigned i=0;i<256;i++){float l,r;vc_render_stereo(&s,&l,&r,1);emit(l);emit(r);}
 }

 /* Noise/ring do not follow oscillator volume, including at zero. */
 for(unsigned volume=0;volume<128;volume++){
  VCSynth a,b;vc_init(&a,44100);vc_init(&b,44100);
  vc_param(&a,VC_NOISE_VOLUME,70);vc_param(&b,VC_NOISE_VOLUME,70);
  vc_param(&a,VC_RING_VOLUME,39);vc_param(&b,VC_RING_VOLUME,39);
  vc_param(&b,VC_SAT_DRIVE,volume);vc_note_on(&a,60,127);vc_note_on(&b,60,127);
  for(unsigned i=0;i<256;i++){
   float al,ar,bl,br;vc_mix_sources(&a,&a.voice[0],.5f,-.25f,0,0,&al,&ar);vc_mix_sources(&b,&b.voice[0],.5f,-.25f,0,0,&bl,&br);
   assert(al==bl&&ar==br);assert(a.voice[0].noise.rng==b.voice[0].noise.rng);
  }
 }
}
