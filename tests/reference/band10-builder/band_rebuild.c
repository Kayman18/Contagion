#include "band_rebuild.h"
#include <string.h>
#include <float.h>
static const unsigned harmonics[14]={128,96,64,48,32,24,16,12,8,6,4,3,2,1};
int vc_band_begin(VCBandRebuild *s,const double *c,const double *sines,float peak,unsigned band,float output[2049]){
 if(!s||!c||!sines||!output||band>=14||!(peak<=FLT_MAX)||peak<0)return 0;
 memset(s,0,sizeof(*s));s->coefficients=c;s->sines=sines;s->peak=peak;s->output=output;s->harmonics=harmonics[band];s->harmonic=1;s->active=1;return 1;
}
int vc_band_service(VCBandRebuild *s,unsigned budget){
 if(!s||!s->output)return -1;
 while(budget--&&s->active){
  unsigned h=s->harmonic,j=(h*s->sample)&2047;
  double a=s->coefficients[h-1],b=s->coefficients[128+h-1];
  if(a!=0||b!=0)s->accumulator+=a*s->sines[j]+b*s->sines[(j+512)&2047];
  s->operations++;
  if(++s->harmonic>s->harmonics){
   /* Preserve the original rounding before normalization. */
   float value=(float)s->accumulator;
   s->output[s->sample]=s->peak>0?(float)((double)value/s->peak):value;
   s->accumulator=0;s->harmonic=1;
   if(++s->sample==2048){s->output[2048]=s->output[0];s->active=0;}
  }
 }
 return s->active?0:1;
}
