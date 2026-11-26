/* Include the engine so both paths use exactly the same unchanged raw/step
 * helpers; reference.inc retains the original TEST05 division/edge decisions. */
#include <math.h>
#include "../engine/virooos.c"
#include "oscillator-test05-reference.inc"
#include <assert.h>
#include <stdio.h>
static uint32_t rng=0x654321;
static float random_unit(void){rng=rng*1664525u+1013904223u;return (rng>>8)*(1.f/16777216.f);}
int main(void){
 unsigned samples=0;
 for(int trial=0;trial<2048;trial++){
  VCOsc a={0},b;float inc=trial%7==0?0:trial%7==1?1.f/16777216.f:.45f*random_unit();
  vc_osc_setup(&a,trial&1,(float)(trial%128),.75f*random_unit(),inc);a.phase=random_unit();b=a;
  for(int n=0;n<1024;n++){
   float sync=n%11==0?random_unit():-1;
   if(n%127==0)sync=0;if(n%131==0)sync=1;
   float x=vc_osc_next(&a,inc,sync),y=vc_osc_next_test05(&b,inc,sync);
   assert(isfinite(x)&&!memcmp(&x,&y,sizeof(x))&&!memcmp(&a,&b,sizeof(a)));samples++;
  }
 }
 /* Exact and neighboring edges at dyadic increments. */
 for(int k=2;k<24;k++)for(int edge=0;edge<3;edge++){
  float inc=ldexpf(1,-k),p=1-inc;
  if(edge==0)p=nextafterf(p,0);if(edge==2)p=nextafterf(p,1);
  VCOsc a={0},b;vc_osc_setup(&a,0,127,.5f,inc);a.phase=p;b=a;
  for(int j=0;j<16;j++){float x=vc_osc_next(&a,inc,-1),y=vc_osc_next_test05(&b,inc,-1);assert(!memcmp(&x,&y,4)&&!memcmp(&a,&b,sizeof(a)));samples++;}
 }
 VCSynth synth;vc_profile_prepare(&synth);unsigned before=0,after=0;
 for(unsigned j=0;j<44100;j++)for(int v=0;v<VC_VOICES;v++)for(int k=0;k<2;k++){
  VCOsc *o=&synth.voice[v].osc[k];float inc=synth.voice[v].inc[k];
  if(o->saw_gain>0){before++;after+=(1-o->phase<=inc+inc);}
  if(o->pulse_gain>0){float q=o->phase+o->duty;if(q>=1)q-=1;before++;after+=(1-q<=inc+inc);}
  vc_osc_next(o,inc,-1);
 }
 printf("Fixed TEST05 chord, one second: oscillator edge divisions %u -> %u (excludes sync/sub/setup divisions)\n",before,after);
 printf("PASS: %u oscillator outputs and states bit-identical to TEST05 across morphs, waves, widths, sync, tiny/zero increments and boundary cases\n",samples);
}
