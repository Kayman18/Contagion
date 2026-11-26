/* Captured source Ge: filter-1 saturation, including its table extrapolation. */
#include "virooos.h"
#define FM_INLINE_ATTR __attribute__((always_inline))
#include "fastmath.h"
#include "saturation.inc"
static const float rate_octaves[8]={-1.464614941f,-0.6022362702f,-0.7274218215f,-0.6975800640f,-0.6436330178f,-0.7384540596f,-0.5896238781f,-0.5738003944f};
void vc_sat_setup(VCSaturation *s,float sr,unsigned curve,float level,float semitones){
 if(curve>14)curve=14;
 if(level<64)level=64;
 if(level>127)level=127;
 unsigned follow=curve==10||curve==12||curve==14;
 if(s->configured&&s->sample_rate==sr&&s->curve==curve&&s->level==level&&(!follow||s->semitones==semitones))return;
 s->configured=1;s->sample_rate=sr;s->curve=curve;s->level=level;s->semitones=semitones;
 if(curve>=1&&curve<=8){
  unsigned i=0;while(i<3&&level>levels[i+1])i++;
  s->lo=curves[curve-1][i];s->hi=curves[curve-1][i+1];s->mix=fm_minf(1,(level-levels[i])/(levels[i+1]-levels[i]));
 }else if(curve==9||curve==10){
  unsigned at=(unsigned)level;
  float hz;
  if(level==at)hz=rates[at];
  else {float t=(level-64)/8;unsigned i=(unsigned)t;float lo=rates[64+8*i];hz=lo*fm_exp2f(rate_octaves[i]*(t-i));}
  hz*=(follow?fm_exp2f(semitones/12):1);s->period=fm_maxf(1,sr/hz);
 }else if(curve>=11){
  unsigned at=(unsigned)level;
  float hz=level==at?(curve<=12?lows[at]:highs[at]):fm_maxf(20,fm_minf(22968,711.4f*fm_exp2f(1.290424404f*((curve<=12?96-level:level-96)/8))));
  hz*=(follow?fm_exp2f(.52f*semitones/12):1);
  s->coef=1-fm_expf(-6.283185307179586f*fm_minf(hz,sr*.49f)/sr);
 }
}
#include "saturation_inline.h"
float vc_sat_next(VCSaturation *s,float x){return vc_sat_next_inline(s,x);}
