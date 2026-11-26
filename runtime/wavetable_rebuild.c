#include "wavetable_rebuild.h"
#include <string.h>
#include <float.h>
static const unsigned harmonics[14]={128,96,64,48,32,24,16,12,8,6,4,3,2,1};
int vc_band_begin(VCBandRebuild *s,const double *c,const double *sines,float peak,unsigned band,float output[2049]){
 if(!s||!c||!sines||!output||band>=14||!(peak<=FLT_MAX)||peak<0)return 0;
 memset(s,0,sizeof(*s));s->coefficients=c;s->sines=sines;s->peak=peak;s->output=output;s->harmonics=harmonics[band];s->harmonic=1;s->active=1;return 1;
}
int vc_band_service(VCBandRebuild *s,unsigned budget){
 if(!s||!s->output)return -1;
 while(budget&&s->active){
  unsigned h=s->harmonic,n=32-s->tile_index;if(n>budget)n=budget;
  double a=s->coefficients[h-1],b=s->coefficients[128+h-1];
  unsigned end=s->tile_index+n;
  /* Reuse coefficients across 32 samples; each sample still accumulates
   * harmonics in precisely the original order. Budget can split any tile. */
  if(a!=0||b!=0)for(unsigned i=s->tile_index;i<end;i++){
   unsigned j=(h*(s->sample+i))&2047;
   s->accumulator[i]+=a*s->sines[j]+b*s->sines[(j+512)&2047];
  }
  s->tile_index=end;s->operations+=n;budget-=n;
  if(end==32){
   s->tile_index=0;
   if(++s->harmonic>s->harmonics){
    for(unsigned i=0;i<32;i++){
     float value=(float)s->accumulator[i];
     s->output[s->sample+i]=s->peak>0?(float)((double)value/s->peak):value;
     s->accumulator[i]=0;
    }
    s->harmonic=1;s->sample+=32;
    if(s->sample==2048){s->output[2048]=s->output[0];s->active=0;}
   }
  }
 }
 return s->active?0:1;
}
