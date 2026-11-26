/* Body of vc_sat_next, for inlining into the voice kernel. */
#ifndef VC_SATURATION_INLINE_H
#define VC_SATURATION_INLINE_H
#ifndef FM_INLINE_ATTR
#define FM_INLINE_ATTR __attribute__((always_inline))
#endif
#include "fastmath.h"
static inline __attribute__((always_inline)) float vc_sat_next_inline(VCSaturation *s,float x){
 if(!s->curve)return x;
 if(s->curve<=8){
  float p=fm_maxf(-1.5f,fm_minf(1.5f,x))*8+8;
  unsigned i=p<0?0:p>=16?15:(unsigned)p;float fraction=p-i;
  float a=s->lo[i]+fraction*(s->lo[i+1]-s->lo[i]);
  return a+s->mix*(s->hi[i]+fraction*(s->hi[i+1]-s->hi[i])-a);
 }
 if(s->curve<=10){s->counter+=1;if(s->counter>=s->period){s->counter-=s->period;s->hold=x;}return s->hold;}
 s->lp+=s->coef*(x-s->lp);return s->curve<=12?s->lp:x-s->lp;
}
#endif
