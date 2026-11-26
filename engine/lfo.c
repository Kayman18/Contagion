/* Captured source class K: standard shapes 0..5 and source wavetable shapes 6..67, contour and one-shot behavior.
 * Random draws use an independent deterministic 24-bit-uniform xorshift stream. */
#include "virooos.h"
#define FM_INLINE_ATTR __attribute__((always_inline))
#include "fastmath.h"
#include <string.h>
#ifndef VC_ACTIVE_LFO
#include "lfo_waves.inc"
#else
/* Updated only after the audio thread acknowledges mute. */
static float lfo_active_tables[3][2049];
static unsigned lfo_active_shapes[3];
void vc_lfo_active_reset(void){memset(lfo_active_shapes,0,sizeof(lfo_active_shapes));}
unsigned vc_lfo_active_shape(unsigned slot){return slot<3?lfo_active_shapes[slot]:0;}
int vc_lfo_active_set(unsigned slot,unsigned shape,const float *table){
 if(slot>=3||shape>VC_LFO_MAX_SHAPE)return 0;
 if(shape>=6){
  if(table)memcpy(lfo_active_tables[slot],table,sizeof(lfo_active_tables[slot]));
  else if(shape<46)return 0;
 }
 lfo_active_shapes[slot]=shape;return 1;
}
#endif
#ifdef VC_DYNAMIC_WAVES
static const float *extra_tables;
void vc_lfo_set_extra_tables(const float *tables){extra_tables=tables;}
#endif
static inline __attribute__((always_inline)) float rnd(VCLfo *l){uint32_t x=l->rng;if(!x)x=1;x^=x<<13;x^=x>>17;x^=x<<5;l->rng=x;return (x>>8)*(1.f/16777216.f);}
static inline __attribute__((always_inline)) float clip(float x){return x>1?1:x< -1?-1:x;}
void vc_lfo_init(VCLfo *l,float sr,uint32_t seed){memset(l,0,sizeof(*l));l->sample_rate=sr;l->rng=seed;l->phase=rnd(l);l->next=rnd(l)*2-1;l->rate=1;}
void vc_lfo_config(VCLfo *l,float rate,unsigned shape,float contour,unsigned env){l->rate=rate;l->shape=shape>VC_LFO_MAX_SHAPE?VC_LFO_MAX_SHAPE:shape;l->contour=contour;l->env_mode=env;if(!env)l->done=0;}
float vc_lfo_deviation(VCLfo *l){return 1+(rnd(l)-.5f)*.04f;}
void vc_lfo_trigger(VCLfo *l,unsigned phase){
 if(phase)l->phase=(phase-1)/128.f;else if(l->env_mode)l->phase=0;
 if(phase||l->env_mode){l->current=l->shape==5?0:rnd(l)*2-1;l->next=rnd(l)*2-1;}
 l->start=l->phase;l->cycles=0;l->done=0;
}
float vc_lfo_shape(const VCLfo *l,float phase){
 float t=l->contour;
 switch(l->shape){
 case 0:{float n=-fm_cosf(6.283185307179586f*phase);if(t<0){float r=phase<.5f?4*phase-1:3-4*phase;return n+(-t/64)*(r-n);}return t>0?clip(n/(1-t/64)):n;}
 case 1:{float n=.5f+t/128;return n<=0?1-2*phase:phase<n?2*phase/n-1:1-2*(phase-n)/(1-n);}
 case 2:{float n=1-2*phase;if(t>0)return clip(n/(1-t/64));if(t<0){static const float g[]={.0001f,.28f,1.2f,2.2f,3.9f};float at=fm_minf(4,-t/16);unsigned i=(unsigned)at;if(i>3)i=3;float k=g[i]+(at-i)*(g[i+1]-g[i]),end=fm_expf(-k);return 2*(fm_expf(-k*phase)-end)/(1-end)-1;}return n;}
 case 3:return phase<.5f+t/128?1:-1;
 case 4:return l->current;
 case 5:return l->current+(l->next-l->current)*phase;
 default:{
  /* Source Be(table, phase * (1 - .9 * max(0, contour / 64))). */
  float at=phase*(1-.9f*fm_maxf(0,t/64))*2048;
  unsigned i=(unsigned)at;float fraction=at-i;i&=2047;
  const float *wave;
#ifdef VC_DYNAMIC_WAVES
  if(l->shape>=46&&extra_tables){wave=extra_tables+(l->shape-46)*2049;}else
#endif
#ifdef VC_ACTIVE_LFO
  {unsigned slot=0;while(slot<3&&lfo_active_shapes[slot]!=l->shape)slot++;
   if(slot==3)return 0;wave=lfo_active_tables[slot];}
#else
  wave=vc_lfo_waves[l->shape-6];
#endif
  return wave[i]+(wave[i+1]-wave[i])*fraction;
 }
 }
}
float vc_lfo_tick(VCLfo *l,unsigned frames){
 if(!(l->env_mode&&l->done)){
  float step=l->rate*frames/l->sample_rate;l->phase+=step;l->cycles+=step;
  if(l->phase>=1){l->phase-=fm_floorf(l->phase);l->current=l->shape==5?l->next:rnd(l)*2-1;l->next=rnd(l)*2-1;}
  if(l->env_mode&&l->cycles>=1){l->phase=l->start;l->done=1;}
 }
 float x=vc_lfo_shape(l,l->phase);l->value=l->env_mode?1+x:x;return l->value;
}

/* Original per-voice rate: clock divisions bypass key tracking and deviation.
 * Calculate only on note/parameter changes; ticking does not need exponentials. */
float vc_lfo_rate(float base,int note,unsigned tracking,float deviation,unsigned clock,unsigned tempo){
 static const float divisions[22]={0,1.f/64,1.f/32,1.f/16,1.f/8,1.f/4,1.f/2,3.f/64,3.f/32,3.f/16,3.f/8,1.f/24,1.f/12,1.f/6,1.f/3,2.f/3,3.f/4,1,2,4,8,16};
 if(clock>21)clock=21;
 if(tempo>127)tempo=127;
 if(tracking>127)tracking=127;
 if(clock)return 1/(divisions[clock]*4*(60.f/(63+tempo)));
 if(!tracking)return base*deviation; /* preserve TEST14 arithmetic */
 return base*fm_exp2f(((note-60)/12.f)*(tracking/128.f))*deviation;
}
