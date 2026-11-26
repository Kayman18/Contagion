/* Original FX WASM functions 80/92/91 translated to native float arithmetic.
 * Table reconstruction is offline; no allocation or double arithmetic here. */
#include "distortion.h"
#include <string.h>

#include "distortion.inc"
static inline __attribute__((always_inline)) uint32_t magnitude(float x){union{float f;uint32_t u;}v={x};return v.u&0x7fffffff;}
static inline __attribute__((always_inline)) float unit(float x){return magnitude(x)>=0x7f800000?0:x>0x1.fffffcp-1f?0x1.fffffcp-1f:x< -1?-1:x;}
static inline __attribute__((always_inline)) float floor_positive(float x){return x>=8388608.f||magnitude(x)>=0x7f800000?x:(float)(uint32_t)x;}
static float interp(const float *t,unsigned level){unsigned i=level>>4;float f=(level&15)/16.f;return t[i]+f*(t[i+1]-t[i]);}
void vc_distortion_reset(VCDistortion *s){memset(s,0,sizeof(*s));}
void vc_distortion_set(VCDistortion *s,unsigned mode,unsigned level){
 if(mode>11)mode=11;if(level>127)level=127;
 s->mode=mode;s->level=level;s->drive=interp(dist_drive,level);
 if(mode>=1&&mode<=4)s->gain=interp(dist_gain+(mode-1)*9,level);
 else if(mode==5)s->gain=interp(dist_gain5,level)*5.3125f;
 else if(mode==6)s->gain=(level/128.f+1)*interp(dist_gain6,level);
 s->quantum=dist_quantum[level];s->step=dist_step[level];
 if(mode==10||mode==11){float p=(mode==10?1-level/128.f:level/128.f)*31;unsigned i=(unsigned)p;float f=p-i;unsigned j=i<31?i+1:31;
  s->coef_a=dist_filter_coeff[i*2]+f*(dist_filter_coeff[j*2]-dist_filter_coeff[i*2]);
  s->coef_b=dist_filter_coeff[i*2+1]+f*(dist_filter_coeff[j*2+1]-dist_filter_coeff[i*2+1]);
 }
}
/* Only ambiguous half-step boundaries need full quotient precision. Recreate
 * binary64 division with integer remainder steps, then round away from zero.
 * The common path remains float. q is the exact original binary64 quantum.
 * Called with positive normal x and a quotient below 2^23. */
static VC_DIST_HOT float quantize_boundary(float x,unsigned level){
 union{float f;uint32_t u;}v={x};
 uint64_t q=(uint64_t)dist_quantum_bits[level*2+1]<<32|dist_quantum_bits[level*2];
 uint64_t den=(q&UINT64_C(0xfffffffffffff))|UINT64_C(0x10000000000000);
 uint64_t num=(uint64_t)((v.u&0x7fffff)|0x800000)<<29;
 int exp=(int)((v.u>>23)&255)-127-((int)((q>>52)&2047)-1023);
 if(num<den){num<<=1;exp--;}
 uint64_t rem=num-den,mant=1;
 for(unsigned i=0;i<52;i++){rem<<=1;mant<<=1;if(rem>=den){rem-=den;mant|=1;}}
 uint64_t twice=rem<<1;
 if(twice>den||(twice==den&&(mant&1)))mant++;
 if(mant==UINT64_C(0x20000000000000)){mant>>=1;exp++;}
 if(exp< -1)return 0;
 unsigned shift=(unsigned)(52-exp);
 return (float)(uint32_t)((mant+(UINT64_C(1)<<(shift-1)))>>shift);
}
float vc_distortion_next(VCDistortion *s,float x,unsigned ch){
 if(ch>1)return 0;
 unsigned mode=s->mode;
 if(!mode)return x;
 if(mode<=4){
  if(magnitude(x)>0x7f800000)x=0;x*=s->drive;x=x>1?1:x< -1?-1:x;
  float p=x*256+256;unsigned i=(unsigned)p,j=i<512?i+1:512;float f=p-i;const float *t=dist_curves+(mode-1)*513;
  return s->gain*(t[i]+f*(t[j]-t[i]));
 }
 if(mode==5){x=x*s->drive*s->drive*512;return s->gain*(x>1?1:x< -1?-1:x);}
 if(mode==6){
  if(magnitude(x)>0x7f800000)x=0;float p=x*s->drive*2048+64;p=p>129?129:p<0?0:p;
  unsigned i=(unsigned)p;float f=p-i;if(i>127)i=127;
  return s->gain*(dist_curve6[i]*(1-f)+f*dist_curve6[i+1])*(1.f/2048);
 }
 if(mode==8){
  float ax=x<0?-x:x,p=ax*dist_quantum_inverse[s->level];
  float n=floor_positive(p+.5f),fraction=p-floor_positive(p);
  float distance=fraction>.5f?fraction-.5f:.5f-fraction;
  if(p<8388608.f&&distance<=p*0x1p-21f)n=quantize_boundary(ax,s->level);
  if(x<0)n=-n;
  return s->quantum*n*0x1.7c9p-2f;
 }
 if(mode==9){s->counter[ch]+=s->step;if(s->counter[ch]&0x800000){s->counter[ch]&=0x7fffff;s->held[ch]=unit(x);}return s->held[ch];}
 float old=s->previous[ch];
 if(mode==7){x=unit(x*((s->level/256.f+.5f)*2));if(x>0)x*=(1-s->level/64.f)*2;s->previous[ch]=unit(x);
  return s->filtered[ch]=unit(s->filtered[ch]*0x1.fea69p-1f+x*0x1.ff5348p-1f+old*-0x1.ff5348p-1f);
 }
 x=unit(x);s->previous[ch]=x;float a=s->coef_a,b=s->coef_b;
 if(mode==10)return s->filtered[ch]=unit(((a-b)*2)*s->filtered[ch]+(b*2)*x+(b*2)*old);
 return s->filtered[ch]=unit(((a-b)*2)*s->filtered[ch]+(a*2)*x+old*a*-2);
}
