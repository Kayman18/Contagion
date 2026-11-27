#include "eq.h"
#include <string.h>
#include "eq.inc"
#include "eq_blocks.inc"
static unsigned limit(unsigned x){return x>127?127:x;}
static inline __attribute__((always_inline)) float clip(float x){union{float f;uint32_t u;}v={x};return (v.u&0x7fffffff)>=0x7f800000?0:x>0x1.fffffcp-1f?0x1.fffffcp-1f:x< -1?-1:x;}
/* Dekker split, exactly as TEST's add_product computed it. */
static inline __attribute__((always_inline)) void split(float a,float *out){float ca=4097*a,ah=ca-(ca-a);out[0]=ah;out[1]=a-ah;}
static int32_t gain26(const int32_t *t,unsigned n){unsigned i=n>>3;return t[i]*8+(int32_t)(n&7)*(t[i+1]-t[i]);}
/* Decode one indexed block of second differences. Only coefficient
 * updates call this path. The per-sample filter still uses its cached state. */
static void mid_pair(unsigned index, int32_t pair[2]){
 const uint8_t *next=eq_blocks+eq_block_offsets[index/32];
 for(unsigned lane=0;lane<2;lane++){
  int32_t value=0,delta=0;
  for(unsigned i=0;i<32;i++){
   unsigned encoded=0,shift=0,byte;
   do{byte=*next++;encoded|=(byte&127u)<<shift;shift+=7;}while(byte&128u);
   delta+=(int32_t)(encoded>>1)^-(int32_t)(encoded&1u);
   value+=delta;
   if(i==index%32u){pair[lane]=value;if(lane==1)return;}
  }
 }
}
static __attribute__((noinline)) int64_t product(int32_t a,int32_t b){return (int64_t)a*b;}
static float rounded23(int64_t n){int64_t v=n<0?-((-n+(INT64_C(1)<<25))>>26):(n+(INT64_C(1)<<25))>>26;uint32_t u=(uint32_t)v&0xffffff;return ((int32_t)(u<<8)>>8)*(1.f/8388608);}
static void shelf(float *out,unsigned gain,unsigned freq,unsigned high){
 float a=gain26(eq_gain_a,gain)*(1.f/67108864),b=gain26(eq_gain_b,gain)*(1.f/67108864);
 float correction=b>.5f?(b-.5f)*128:0;
 float position=high?freq*.375f+81-correction:correction+freq*.375f+9;
 position=position*.25f-1;if(position<0)position=0;if(position>31)position=31;
 unsigned i=(unsigned)position,j=i<31?i+1:31;float f=position-i;
 float ca=eq_frequency[i*2]+f*(eq_frequency[j*2]-eq_frequency[i*2]);
 float cb=eq_frequency[i*2+1]+f*(eq_frequency[j*2+1]-eq_frequency[i*2+1]);
 if(b>.5f)b=.5f;
 out[0]=(a+a)*(high?ca:cb)+b;out[2]=ca-cb;
 out[1]=((high?b:-b)*(ca-cb)+a*(high?ca:cb))*(high?-2:2);
}
static float norm(unsigned n){unsigned i=n>>3;return (eq_norm[i]+(n&7)*.125f*(eq_norm[i+1]-eq_norm[i]))*8;}
void vc_eq_reset(VCEQ *s){memset(s,0,sizeof(*s));}
void vc_eq_set(VCEQ *s,unsigned lg,unsigned lf,unsigned mg,unsigned mf,unsigned mq,unsigned hg,unsigned hf){
 lg=limit(lg);lf=limit(lf);mg=limit(mg);mf=limit(mf);mq=limit(mq);hg=limit(hg);hf=limit(hf);
 shelf(s->low,lg,lf,0);shelf(s->high,hg,hf,1);s->normalization=norm(mg)*norm(lg)*norm(hg);
 int64_t a=gain26(eq_gain_a,mg),b=gain26(eq_gain_b,mg);if(b>33554432)b=33554432;
 int32_t pair[2];mid_pair(mq*128+mf,pair);
 int64_t e=pair[0],g=pair[1],v=product((int32_t)a,pair[1]+4194304);
 s->mid[0]=rounded23(b*8388608+v);s->mid[1]=rounded23(product((int32_t)e,(int32_t)(-2*b)));s->mid[2]=rounded23(product((int32_t)(-2*b),(int32_t)g)-v);
 s->mid[3]=(float)e*(1.f/8388608);s->mid[4]=(float)g*(1.f/8388608);
 for(unsigned i=0;i<5;i++)split(s->mid[i],s->mid_split[i]);
}
/* Compensated products and sum keep high-Q mid feedback below the measured
 * float-only drift without software double operations in the sample loop. */
/* add_product with both Dekker splits supplied: TEST computed the same
 * splits of the same values inside every call. */
static inline __attribute__((always_inline)) void add_product(float *sum,float *error,float a,const float *as,float b,const float *bs){
 float p=a*b,ah=as[0],al=as[1],bh=bs[0],bl=bs[1];
 float pe=((ah*bh-p)+ah*bl+al*bh)+al*bl;
 float t=*sum+p,z=t-*sum;*error+=((*sum-(t-z))+(p-z))+pe;*sum=t;
}
float vc_eq_next(VCEQ *s,float x,unsigned channel){
 if(channel>1)return 0;float *h=s->history[channel];float (*hs)[2]=s->history_split[channel];x=clip(x);
 float x1=h[0],x2=h[1],y1=h[2],y2=h[3];h[1]=x1;h[3]=y1;h[0]=x;
 float xs[2];split(x,xs);
 float t=0,error=s->mid_error[channel][1]*s->mid[4]+s->mid_error[channel][0]*s->mid[3];
 add_product(&t,&error,y2,hs[3],s->mid[4],s->mid_split[4]);add_product(&t,&error,y1,hs[2],s->mid[3],s->mid_split[3]);
 add_product(&t,&error,x2,hs[1],s->mid[2],s->mid_split[2]);add_product(&t,&error,s->mid[0],s->mid_split[0],x,xs);
 add_product(&t,&error,x1,hs[0],s->mid[1],s->mid_split[1]);
 hs[1][0]=hs[0][0];hs[1][1]=hs[0][1];hs[0][0]=xs[0];hs[0][1]=xs[1];hs[3][0]=hs[2][0];hs[3][1]=hs[2][1];
 t+=t;error+=error;float combined=t+error;float residual=error-(combined-t);
 x=clip(combined);s->mid_error[channel][1]=s->mid_error[channel][0];s->mid_error[channel][0]=x==combined?residual:0;h[2]=x;split(x,hs[2]);
 float in=h[4],out=h[5];h[4]=x;t=s->low[2]*out+s->low[0]*x+s->low[1]*in;x=clip(t+t);h[5]=x;
 in=h[6];out=h[7];h[6]=x;t=s->high[2]*out+s->high[0]*x+s->high[1]*in;x=clip(t+t);h[7]=x;
 return clip(s->normalization*x);
}
void vc_eq_commit_channel(VCEQ *to,const VCEQ *from,unsigned ch){
 if(ch>1)return;
 memcpy(to->history[ch],from->history[ch],sizeof(to->history[ch]));
 memcpy(to->mid_error[ch],from->mid_error[ch],sizeof(to->mid_error[ch]));
 memcpy(to->history_split[ch],from->history_split[ch],sizeof(to->history_split[ch]));
}


#include "boost.inc"
void vc_boost_set(VCBoost *s,unsigned intensity,unsigned tune){
 intensity=limit(intensity);tune=limit(tune);s->enabled=intensity;
 if(!intensity){s->coef[0]=.5f;s->coef[1]=s->coef[2]=0;return;}
 const uint8_t *p=boost_data+boost_offsets[tune];int32_t value[2]={0,0},delta[2]={0,0};
 for(unsigned i=1;i<=intensity;i++)for(unsigned k=0;k<2;k++){
  uint32_t n=0;unsigned shift=0,b;
  do{b=*p++;n|=(uint32_t)(b&127)<<shift;shift+=7;}while(b&128);
  delta[k]+=(n&1)?-(int32_t)(n>>1)-1:(int32_t)(n>>1);value[k]+=delta[k];
 }
 s->coef[0]=value[0]*(1.f/8388608);s->coef[1]=value[1]*(1.f/8388608);s->coef[2]=boost_feedback[tune]*(1.f/8388608);
}
float vc_boost_next(VCBoost *s,float x,unsigned ch){
 if(ch>1)return 0;
 float *h=s->history[ch],input=clip(x);
 if(!s->enabled){h[0]=h[1]=input;return x;}
 float t=s->coef[2]*h[1]+(s->coef[0]*input+h[0]*s->coef[1]);
 h[0]=input;h[1]=clip(t+t);return h[1];
}
