#include "virooos.h"
#ifdef VC_DUAL_RENDER
#include "../app/dual_core.h"
#endif
#ifdef VC_BAND_CACHE
#include "../runtime/wavetable_voice.h"
#ifndef VC_DYNAMIC_WAVES
#error VC_BAND_CACHE requires VC_DYNAMIC_WAVES
#endif
#endif
#define FM_INLINE_ATTR __attribute__((always_inline))
#include "fastmath.h"
#define floorf fm_floorf
#define fabsf fm_fabsf
#define fminf fm_minf
#define fmaxf fm_maxf
#define tanf fm_tanf
#define expf fm_expf
#define exp2f fm_exp2f
#include <string.h>
#include "tables.inc"
#include "saturation_inline.h"
#include "unison.inc"
#ifdef VC_DYNAMIC_WAVES
#include "wave_cache.h"
#endif
#define VC_PI 3.14159265358979323846f
static inline __attribute__((always_inline)) float vc_clip(float x,float a,float b){return x<a?a:x>b?b:x;}
static inline __attribute__((always_inline)) float vc_lookup(const float *t,float x){x=vc_clip(x,0,128);int i=(int)x;if(i>=128)return t[128];return t[i]+(x-i)*(t[i+1]-t[i]);}
static inline __attribute__((always_inline)) float vc_blep_before(float x){return x*x+2*x+1;}
static inline __attribute__((always_inline)) float vc_blep_after(float x){return 2*x-x*x-1;}
/* Original He sub oscillator: square with polyBLEP, or source triangle. */
static inline __attribute__((always_inline)) float vc_sub_blep(float phase,float inc){
 if(phase<inc)return vc_blep_after(phase/inc);
 if(phase>1-inc)return vc_blep_before((phase-1)/inc);
 return 0;
}
static inline __attribute__((always_inline)) float vc_sub_next_inline(VCSub *o,float inc,int triangle){
 float p=o->phase,x;
 if(triangle)x=3.97f*.669f*(p<.5f?1-4*p:4*p-3);
 else {x=p<.5f?1:-1;if(inc>0){x+=vc_sub_blep(p,inc);x-=vc_sub_blep(p>=.5f?p-.5f:p+.5f,inc);}x*=.669f;}
 o->phase=p+inc;if(o->phase>=1)o->phase-=1;
 return x;
}
float vc_sub_next(VCSub *o,float inc,int triangle){return vc_sub_next_inline(o,inc,triangle);}
/* Original Ue colour filter/level, with a local uniform PRNG replacing
 * browser Math.random(). Each voice has independent state. */
void vc_noise_setup(VCNoise *n,float sr,unsigned color){
 if(color>127)color=127;
 if(n->configured&&n->color==(int)color&&n->sample_rate==sr)return;
 n->configured=1;n->color=(int)color;n->sample_rate=sr;n->gain=vc_noise_gain[color];
 n->coef=1-expf(-2*VC_PI*fminf(vc_noise_hz[color],sr*.49f)/sr);
}
static inline __attribute__((always_inline)) float vc_noise_next_inline(VCNoise *n){
 uint32_t x=n->rng;if(!x)x=1;x^=x<<13;x^=x>>17;x^=x<<5;n->rng=x;
 float white=((x>>8)*(1.f/8388608.f)-1)*.744f;
 if(n->color==64)return white;
 n->lp+=n->coef*(white-n->lp);
 return n->color<64?n->gain*n->lp:white-(1-n->gain)*n->lp;
}
float vc_noise_next(VCNoise *n){return vc_noise_next_inline(n);}
static inline __attribute__((always_inline)) int vc_osc_band(float inc){
 static const int h[14]={128,96,64,48,32,24,16,12,8,6,4,3,2,1};
 float maxh=inc>0?.5f/inc:128;for(int i=0;i<14;i++)if(h[i]<=maxh)return i;return 13;
}
static inline __attribute__((always_inline)) void vc_osc_shape(VCOsc *o,float shape){
 if(shape<64){float t=vc_clip(shape/63.35f,0,1);o->wave_gain=1-t;o->saw_gain=.669f*t;o->pulse_gain=0;}
 else {float t=vc_clip((shape-64)/63,0,1);o->wave_gain=0;o->saw_gain=.669f*(1-t+.747f*t);o->pulse_gain=.669f*.747f*t;}
}
void vc_osc_setup(VCOsc *o,int wave,float shape,float width,float inc){
 o->wave=wave<0?0:wave>=VC_WAVES?VC_WAVES-1:wave;
 vc_osc_shape(o,shape);
 float d=1-vc_clip(width,0,.75f);o->duty=d>=1?0:d;
 o->band=vc_osc_band(inc);
#if defined(VC_BAND_CACHE)
 o->tables=0;if(!o->table)o->table=vc_band_silence;
#elif defined(VC_DYNAMIC_WAVES)
 o->tables=vc_wave_tables((unsigned)o->wave);
 o->table=vc_wave_table((unsigned)o->wave,(unsigned)o->band);
#else
 o->tables=vc_wave_views[o->wave];
 o->table=o->tables[o->band];
#endif
}
static inline __attribute__((always_inline)) float vc_osc_raw(const VCOsc *o,float p){
 float x=0;
 if(o->wave_gain>0){float at=(p-floorf(p))*2048;int i=(int)at;const float *t=o->table;x+=o->wave_gain*(t[i]+(t[i+1]-t[i])*(at-i));}
 x+=o->saw_gain*(2*p-1);
 if(o->pulse_gain>0){float q=p+o->duty;if(q>=1)q-=1;x-=o->pulse_gain*(2*q-1);}
 return x;
}
static inline __attribute__((always_inline)) float vc_osc_step(VCOsc *o,float fraction,float step){float a=step*.5f;o->pending+=a*vc_blep_after(1-fraction);return a*vc_blep_before(-fraction);}
static inline __attribute__((always_inline)) float vc_osc_next_inline(VCOsc *o,float inc,float sync){
 float p=o->phase,x=vc_osc_raw(o,p)+o->pending;o->pending=0;
#ifdef VC_BAND_CACHE
 if(o->table_ramp){x+=o->wave_gain*o->table_correction*(o->table_ramp*(1.f/128.f));o->table_ramp--;}
#endif
 /* Two-increment guard skips division only beyond the edge interval.
  * Original fraction/rounding and sync decisions remain near edges. */
 if(inc>0){float end=sync>=0?sync:1;
  if(o->saw_gain>0){float distance=1-p;if(distance<=inc+inc || end>1){float t=distance/inc;if(t<=end)x+=vc_osc_step(o,t,-2*o->saw_gain);}}
  if(o->pulse_gain>0){float q=p+o->duty;if(q>=1)q-=1;float distance=1-q;if(distance<=inc+inc || end>1){float t=distance/inc;if(t<=end)x+=vc_osc_step(o,t,2*o->pulse_gain);}}}
 if(sync>=0){float q=p+inc*sync;q-=floorf(q);x+=vc_osc_step(o,sync,vc_osc_raw(o,0)-vc_osc_raw(o,q));o->phase=inc*(1-sync);}else o->phase=p+inc;
 if(o->phase>=1||o->phase<0)o->phase-=floorf(o->phase);
 return x;
}
float vc_osc_next(VCOsc *o,float inc,float sync){return vc_osc_next_inline(o,inc,sync);}
void vc_filter_setup(VCFilter *f,float sr,int mode,int poles,float cutoff,float resonance){
 if(f->configured && f->cutoff==cutoff && f->resonance==resonance &&
    f->sample_rate==sr && f->mode==mode && f->poles==poles)return;
 if(f->poles!=poles)f->s3=f->s4=0;
 f->configured=1;f->cutoff=cutoff;f->resonance=resonance;f->sample_rate=sr;
 f->mode=mode;f->poles=poles;float hz=fminf(sr*.49f,vc_lookup(vc_cutoff,cutoff));
 f->g=tanf(VC_PI*hz/sr);f->k=vc_lookup(poles==4?vc_damping4:vc_damping,resonance);
 /* The source's four-pole damping is exponential between its anchors.
  * Integer values retain the existing table; fractional modulation uses the
  * original exponential slope rather than linear interpolation. */
 if(poles==4 && resonance!=(int)resonance){
  static const float slope[4]={-0.01092586558f,-0.02196873366f,-0.023242725f,-0.02356901366f};
  unsigned i=(unsigned)resonance;unsigned segment=resonance<32?0:resonance<64?1:resonance<100?2:3;
  f->k=vc_damping4[i]*expf(slope[segment]*(resonance-i));
 }
 f->gain=mode==2?vc_lookup(vc_bpGain,resonance)*vc_lookup(vc_damping,resonance):mode==3?1:vc_lookup(vc_filterGain,resonance);
 f->inverse=1/(1+f->g*(f->g+f->k));
}
static inline __attribute__((always_inline)) float vc_filter_next_inline(VCFilter *f,float x){
 float g=f->g,k=f->k,a=f->inverse;int m=f->mode;
 if(f->poles==4){float b=(f->s3+g*(x-f->s4))*a,c=f->s4+g*b;f->s3=2*b-f->s3;f->s4=2*c-f->s4;x=m==0?c:m==1?x-k*b-c:m==2?b*k:x-k*b;}
 float b=(f->s1+g*(x-f->s2))*a,c=f->s2+g*b;f->s1=2*b-f->s1;f->s2=2*c-f->s2;
 return (m==0?c:m==1?x-k*b-c:m==2?b:x-k*b)*f->gain;
}
float vc_filter_next(VCFilter *f,float x){return vc_filter_next_inline(f,x);}
void vc_env_setup(VCEnvelope *e,float sr,float a,float d,float s,float r){e->attack_inc=1/fmaxf(1,a*sr);e->decay_coef=expf(-1/fmaxf(1,d*sr));e->sustain=vc_clip(s,0,1);e->release_coef=expf(-1/fmaxf(1,r*sr));e->slope=0;e->fall_coef=1;e->rise_inc=0;}
void vc_env_slope(VCEnvelope *e,float sr,unsigned value){
 if(value>127)value=127;
 e->slope=value<64?-1:value>64?1:0;
 e->fall_coef=value<64?expf(-2.302585092994046f*vc_slope_rate[value]/20/sr):1;
 e->rise_inc=value>64?vc_slope_rate[value]/sr:0;
}
static inline __attribute__((always_inline)) float vc_env_next_inline(VCEnvelope *e){switch(e->stage){
 case 1:e->level+=e->attack_inc;if(e->level>=1){e->level=1;e->stage=2;}break;
 case 2:e->level=e->sustain+(e->level-e->sustain)*e->decay_coef;if(fabsf(e->level-e->sustain)<1e-4f){e->level=e->sustain;e->stage=3;}break;
 case 3:if(e->slope<0)e->level*=e->fall_coef;else if(e->slope>0)e->level=fminf(1,e->level+e->rise_inc);else e->level=e->sustain;break;
 case 4:e->level*=e->release_coef;if(e->level<1e-5f){e->level=0;e->stage=0;}break;
 default:e->level=0;}
 return e->level;
}
float vc_env_next(VCEnvelope *e){return vc_env_next_inline(e);}
/* Ported from source Me()/w() and Ct.control(). Saturation is bypassed in TEST02. */
static inline __attribute__((always_inline)) float vc_route_weight(float t){
    if(t<=32)return 1-.625f*t/32;
    if(t<=48)return .375f-.25f*(t-32)/16;
    return .125f-.125f*(t-48)/16;
}
static inline __attribute__((always_inline)) void vc_pan_pair(float value,float *out){
    float p=vc_clip(value,0,127);if(p>=127)p=128;
    if(p<=64){out[0]=1-.369f*p/64;out[1]=.631f*p/64;}
    else {out[0]=.631f*(128-p)/64;out[1]=1-.369f*(128-p)/64;}
}
/* Original per-voice filter balance; topology switches at the centre. */
static inline __attribute__((always_inline)) void vc_filter_mix(float value,unsigned route,float *mix,unsigned *after){
 float b=vc_clip(value,0,127);if(b>=127)b=128;
 float h=vc_route_weight(b<=64?b:128-b);
 if(route<=1){mix[0]=h;mix[1]=1-h;*after=b>64;}
 else {float a=.5f+.2071f*h,c=.5f*(1-h);mix[0]=b<=64?a:c;mix[1]=b<=64?c:a;*after=0;}
}
void vc_route_setup(VCSynth *s){
    float b=s->param[VC_FILTER_BALANCE];if(b>=127)b=128;
    float h=vc_route_weight(b<=64?b:128-b);
    if(s->param[VC_ROUTING]<=1){s->mix[0]=h;s->mix[1]=1-h;s->mix_after=b>64;}
    else {float a=.5f+.2071f*h,c=.5f*(1-h);s->mix[0]=b<=64?a:c;s->mix[1]=b<=64?c:a;s->mix_after=0;}
    float p=s->param[VC_PAN],d=s->param[VC_ROUTING]==3?s->param[VC_SPREAD]*.5f:0;
    vc_pan_pair(p-d,s->pan);vc_pan_pair(p+d,s->pan+2);
}
static void vc_source_levels(VCSynth *s,VCVoice *v,const float *delta){
 static const unsigned ids[3]={VC_SUB_VOLUME,VC_NOISE_VOLUME,VC_RING_VOLUME};
 for(unsigned i=0;i<3;i++)v->source_gain[i]=vc_lookup(vc_sourceGain,vc_clip(s->param[ids[i]]+(delta?delta[i]:0),0,127));
 float level=vc_clip(s->param[VC_SAT_DRIVE]+(delta?delta[4]:0),0,127);
 v->source_gain[5]=level>=64?1:vc_lookup(vc_punchLevel,2*level)/3.49f;
 v->source_gain[0]*=v->source_gain[5];
 v->punch_gain=s->param[VC_PUNCH]?vc_punchLevel[s->param[VC_PUNCH]]*v->source_gain[5]:0;
 float saturation_key=v->pitch+s->pitch_offset+v->unison_cents/100-60;
#ifdef VC_MATRIX
 saturation_key+=v->transpose_matrix;
#endif
 vc_sat_setup(&v->saturation,s->sample_rate,s->param[VC_SAT_CURVE],level,saturation_key);
 v->source_gain[2]*=1.5f;
 float balance=vc_clip(s->param[VC_BALANCE]+(delta?delta[3]:0),0,127);if(balance>=127)balance=128;
 if(balance<=64){v->source_gain[3]=1-.003219f*balance;v->source_gain[4]=.794f*balance/64;}
 else {v->source_gain[3]=.794f*(128-balance)/64;v->source_gain[4]=1-.003219f*(128-balance);}

}
/* The dual-render kernel inlines every helper; the single-core render keeps
 * the calls (and the TEST74 SRAM size). */
#ifdef VC_DUAL_RENDER
#define VC_NOISE_NEXT vc_noise_next_inline
#define VC_SUB_NEXT vc_sub_next_inline
#define VC_FILTER_NEXT vc_filter_next_inline
#define VC_SAT_NEXT vc_sat_next_inline
#else
#define VC_NOISE_NEXT vc_noise_next
#define VC_SUB_NEXT vc_sub_next
#define VC_FILTER_NEXT vc_filter_next
#define VC_SAT_NEXT vc_sat_next
#endif
static inline __attribute__((always_inline)) void vc_mix_sources_inline(VCSynth *s,VCVoice *v,float raw1,float raw2,float a,float b,float *o1,float *o2){
 float volume=v->source_gain[5];
 if(volume!=1){a*=volume;b*=volume;}
 *o1=a*raw1;*o2=b*raw2;
 /* Original split routing sends noise to filter 2; other routes sum it. */
 if(v->source_gain[1]>0){float noise=v->fm_depth>0&&s->param[VC_FM_MODE]==3?v->fm_noise_sample:VC_NOISE_NEXT(&v->noise);*o2+=noise*v->source_gain[1];}
 if(v->source_gain[0]>0)*o1+=VC_SUB_NEXT(&v->sub,v->inc[0]*.5f,s->param[VC_SUB_SHAPE])*v->source_gain[0];
 if(v->source_gain[2]>0){float ring=v->source_gain[2]*raw1*raw2;*o1+=ring;*o2+=ring;}

}
void vc_mix_sources(VCSynth *s,VCVoice *v,float raw1,float raw2,float a,float b,float *o1,float *o2){vc_mix_sources_inline(s,v,raw1,raw2,a,b,o1,o2);}
/* Source Ct gains interpolate to each new control target over 32 samples.
 * Preserve the existing static pan path when this destination is unused. */
static inline __attribute__((always_inline)) void vc_pan_control_values(VCSynth *s,VCVoice *v,float p,float stereo_width,unsigned enabled){
 enabled|=v->unison_side!=0;float spread=s->param[VC_ROUTING]==3?stereo_width*.5f:0;
 if(!enabled&&!v->pan_active){
  for(unsigned i=0;i<4;i++)v->pan_now[i]=s->pan[i];
  v->pan_initialized=1;return;
 }
 vc_pan_pair(p-spread,v->pan_target);vc_pan_pair(p+spread,v->pan_target+2);
 if(v->unison_side!=0){float width=vc_clip(stereo_width,0,64),side=fabsf(v->unison_side);
  float a=1+side*(.751f+.00686f*width-1),b=1+side*(.751f*(1-width/64)-1);
  for(unsigned i=0;i<4;i++)v->pan_target[i]*=((i&1)?v->unison_side>=0:v->unison_side<=0)?a:b;
 }
 for(unsigned i=0;i<4;i++){
  if(!v->pan_initialized)v->pan_now[i]=v->pan_target[i];
  v->pan_step[i]=(v->pan_target[i]-v->pan_now[i])/VC_CONTROL_FRAMES;
 }
 v->pan_initialized=1;v->pan_active=1;v->pan_remaining=VC_CONTROL_FRAMES;v->pan_stop=!enabled;
}
void vc_lfo2_pan_control(VCSynth *s,VCVoice *v){
 unsigned enabled=(s->param[VC_LFO2_MODE]&&s->param[VC_LFO2_PAN]!=64)||s->param[VC_VEL_PAN]!=64;
 float p=s->param[VC_PAN]+v->velocity_pan+2*((int)s->param[VC_LFO2_PAN]-64)*v->lfo2_mod;
 float spread=s->param[VC_SPREAD];
 vc_pan_control_values(s,v,p,spread,enabled);
}
static inline __attribute__((always_inline)) void vc_route_sample_inline(VCSynth *s,VCVoice *v,float osc1,float osc2,float *left,float *right){
    const float *pan=s->pan;
    if(v->pan_active){
      for(unsigned i=0;i<4;i++)v->pan_now[i]+=v->pan_step[i];
      if(v->pan_remaining&&!--v->pan_remaining){
       for(unsigned i=0;i<4;i++)v->pan_now[i]=v->pan_target[i];
       if(v->pan_stop)v->pan_active=0;
      }
      pan=v->pan_now;
    }
    /* Original 12-sample attack impulse belongs to the filter-1 input side. */
    if(v->punch_at<12){if(v->punch_gain!=0)osc1+=v->punch_gain*vc_punchWave[v->punch_at];v->punch_at++;}
    if(v->lfo_filter_gain!=1){osc1*=v->lfo_filter_gain;osc2*=v->lfo_filter_gain;}
    float a=s->mix[0],b=s->mix[1],x=osc1+osc2,l,r;
    unsigned after=s->mix_after;
#ifdef VC_MATRIX
    if(s->matrix){a=v->filter_mix[0];b=v->filter_mix[1];after=v->filter_mix_after;}
#endif
    int route=s->param[VC_ROUTING];
    if(route<=1){
        float f1=VC_FILTER_NEXT(&v->filter[0],x);
        if(v->saturation.curve)f1=VC_SAT_NEXT(&v->saturation,f1);
        float y=after?VC_FILTER_NEXT(&v->filter[1],a*x+b*f1):a*f1+b*VC_FILTER_NEXT(&v->filter[1],f1);
        l=y*pan[0];r=y*pan[1];
    }else if(route==2){
        float f1=VC_FILTER_NEXT(&v->filter[0],x);
        if(v->saturation.curve)f1=VC_SAT_NEXT(&v->saturation,f1);
        float y=a*f1+b*VC_FILTER_NEXT(&v->filter[1],x);
        l=y*pan[0];r=y*pan[1];
    }else{
        float f1=VC_FILTER_NEXT(&v->filter[0],osc1);
        if(v->saturation.curve)f1=VC_SAT_NEXT(&v->saturation,f1);
        f1*=a;float f2=b*VC_FILTER_NEXT(&v->filter[1],osc2);
        l=f1*pan[0]+f2*pan[2];r=f1*pan[1]+f2*pan[3];
    }
    *left=l;*right=r;
}
void vc_route_sample(VCSynth *s,VCVoice *v,float osc1,float osc2,float *left,float *right){vc_route_sample_inline(s,v,osc1,osc2,left,right);}
/* Original oscillator-2 filter-envelope pitch contribution, in cents. */
static inline __attribute__((always_inline)) void vc_pitch_band(VCVoice *v,unsigned osc,float inc){
 v->inc[osc]=inc;int band=vc_osc_band(inc);
 if(band!=v->osc[osc].band){v->osc[osc].band=band;if(v->osc[osc].tables)v->osc[osc].table=v->osc[osc].tables[band];}
}
/* Cache width offsets, including matrix offsets, so removing a route restores
 * the original width without disturbing oscillator phase or BLEP state. */
static VC_HOT __attribute__((noinline)) void vc_pwm_control(VCSynth *s,VCVoice *v,float pw,float delta1,float delta2,float lfo3_amount){
 unsigned dest=s->param[VC_LFO3_DEST];
 for(unsigned i=0;i<2;i++){
  float width=(i?delta2:delta1)+pw+((i?(dest==4||dest==5):(dest==3||dest==4))?lfo3_amount*v->lfo3_mod:0);
  if(width==v->pw_last[i])continue;
  v->pw_last[i]=width;
  float value=vc_clip(s->param[i?VC_PW2:VC_PW1]+width,-63.5f,127),d=1-.5f*(1-value/127);
  v->osc[i].duty=d>=1?0:d;
 }
}
static inline __attribute__((always_inline)) void vc_pitch_mod_control(VCVoice *v,float cents1,float cents2){
 if(cents1!=v->pitch1_last){v->pitch1_last=cents1;vc_pitch_band(v,0,fminf(.45f,v->pitch1_base_inc*exp2f(cents1/1200)));}
 if(cents2!=v->pitch2_env_last){v->pitch2_env_last=cents2;vc_pitch_band(v,1,fminf(.45f,v->pitch2_base_inc*exp2f(cents2/1200)));}
}
void vc_pitch_env_control(VCSynth *s,VCVoice *v){
 unsigned dest=s->param[VC_LFO3_DEST];
 float pitch3=vc_lfo3Pitch[s->param[VC_LFO3_AMOUNT]]*v->lfo3_mod;
 float cents1=vc_lfoPitch[s->param[VC_LFO_OSC1]]*v->lfo_mod+(dest<=1?pitch3:0);
 float cents2=vc_osc2EnvCents[s->param[VC_OSC2_ENV]]*v->filter_env.level+vc_lfoPitch[s->param[VC_LFO_OSC2]]*v->lfo_mod+(dest==1||dest==2?pitch3:0);
 vc_pitch_mod_control(v,cents1,cents2);
#ifdef VC_MATRIX
 if(s->matrix)return; /* matrix PWM is evaluated after source collection */
#endif
 vc_pwm_control(s,v,v->velocity_pw+vc_lfoPW[s->param[VC_LFO_PW]]*v->lfo_mod,0,0,s->param[VC_LFO3_AMOUNT]);
}
/* Original ge curve morphs both oscillator shapes, without changing phase,
 * duty, table choice or pending BLEP correction. */
void vc_lfo2_osc_control(VCSynth *s,VCVoice *v){
 float delta=((int)s->param[VC_LFO2_SHAPE_AMOUNT]-64)*(127.f/128)*v->lfo2_mod;
 if(delta!=v->lfo2_shape_last){
  v->lfo2_shape_last=delta;
  vc_osc_shape(&v->osc[0],vc_clip(s->param[VC_SHAPE1]+v->velocity_shape[0]+delta,0,127));
  vc_osc_shape(&v->osc[1],vc_clip(s->param[VC_SHAPE2]+v->velocity_shape[1]+delta,0,127));
  v->fm_scale=1/fmaxf(.1f,v->osc[0].wave_gain+v->osc[0].saw_gain/.669f);
 }
}
/* Source oscillator 3 modes 1..3: slave, saw and pulse. Slave shares the
 * oscillator-2 table set but selects its own pitch band. No third cache. */
#ifndef VC_DUAL_RENDER
VC_HOT
#endif
static __attribute__((noinline)) void vc_osc3_control_values(VCSynth *s,VCVoice *v,float cents){
 unsigned mode=s->param[VC_OSC3_MODE];if(!mode)return;
#ifdef VC_MATRIX
 cents-=v->osc2_matrix_detune;
 if(mode==1)cents-=v->osc2_matrix_detune;
 else cents+=v->osc3_matrix_cents;
#endif
 if(cents!=v->osc3_pitch_last){v->osc3_pitch_last=cents;v->inc3=fminf(.45f,v->osc3_base_inc*exp2f(cents/1200));}
 VCOsc *o=&v->osc3,*two=&v->osc[1];
 if(mode==1){o->wave_gain=two->wave_gain;o->saw_gain=two->saw_gain;o->pulse_gain=two->pulse_gain;}
 else vc_osc_shape(o,mode==2?64:mode==3?127:0);
 o->duty=two->duty;o->wave=mode>=4?(int)mode-4:two->wave;
 if(mode<4)o->tables=two->tables;
 o->band=vc_osc_band(v->inc3);
#ifndef VC_BAND_CACHE
 o->table=o->tables?o->tables[o->band]:0;
#endif
}
void vc_osc3_control(VCSynth *s,VCVoice *v){vc_osc3_control_values(s,v,v->pitch2_env_last);}
/* Original St depth curve, including the source's special endpoint at 127. */
static inline __attribute__((always_inline)) float vc_fm_depth_curve(float amount){
 amount=vc_clip(amount,0,127);float x=amount/32;
 return amount>=127?16:amount<32?x:amount<64?1+3*(x-1):amount<96?4+4*(x-2):8+8*(x-3);
}
static inline __attribute__((always_inline)) void vc_fm_apply(VCSynth *s,VCVoice *v){
 float amount=s->param[VC_FM_AMOUNT]+2*((int)s->param[VC_LFO2_FM_AMOUNT]-64)*v->lfo2_mod+2*((int)s->param[VC_FM_ENV]-64)*v->filter_env.level+2*((int)s->param[VC_FM_VELOCITY]-64)*v->velocity;
 v->fm_depth=vc_fm_depth_curve(amount);
}
void vc_fm_control(VCSynth *s,VCVoice *v){vc_fm_apply(s,v);}
static inline __attribute__((always_inline)) float vc_fm_increment_inline(const VCVoice *v,unsigned mode,float phase,float raw1){
 float signal=mode==0?(phase<.5f?1-2*phase:2*phase-1):mode==1?(phase<.5f?1-4*phase:4*phase-3):mode==2?raw1*v->fm_scale:v->fm_noise_sample;
 return v->inc[1]*(1+v->fm_depth*signal);
}
float vc_fm_increment(const VCVoice *v,unsigned mode,float phase,float raw1){return vc_fm_increment_inline(v,mode,phase,raw1);}
/* Recalculate pitch only; glide must not reset envelopes or oscillator phases. */
/* Flash: runs only when a voice pitch changes; SRAM callers use a pointer. */
static __attribute__((noinline)) void vc_pitch_bases(VCSynth *s,VCVoice *v){
 const uint8_t *p=s->param;
    /* Source keyfollow is centered at note 60; value 96 tracks one semitone
     * per key. Keep the old arithmetic at default tuning for exact continuity. */
    float key=v->pitch+s->pitch_offset+v->unison_cents/100-60;
    float off1=key*((int)p[VC_KEYFOLLOW1]-96)/32+vc_clip((int)p[VC_TUNE1]-64,-48,48);
    float off2=key*((int)p[VC_KEYFOLLOW2]-96)/32+vc_clip((int)p[VC_TUNE2]-64,-48,48);
    float hz=440*exp2f((v->pitch-69+s->pitch_offset+v->unison_cents/100+off1)/12);
    float hz2=off2==off1?hz:440*exp2f((v->pitch-69+s->pitch_offset+v->unison_cents/100+off2)/12);

    v->pitch1_base_inc=hz/s->sample_rate;v->pitch1_last=0;
    v->inc[0]=fminf(.45f,v->pitch1_base_inc);
    v->pitch2_base_inc=hz2*exp2f(vc_detune[p[VC_DETUNE]]/1200)/s->sample_rate;
    v->inc[1]=fminf(.45f,v->pitch2_base_inc);v->pitch2_env_last=0;
    v->osc3_gain=0;v->osc3_pitch_last=0;
    if(p[VC_OSC3_MODE]){
      if(p[VC_OSC3_MODE]==1){
        v->osc3_base_inc=hz2*exp2f(-vc_detune[p[VC_DETUNE]]/1200)/s->sample_rate;
        float balance=p[VC_BALANCE]>=127?128:p[VC_BALANCE];
        v->osc3_gain=1.058f*(balance<=64?.794f*balance/64:1-.003219f*(128-balance));
      }else{
        float offset=key*((int)p[VC_KEYFOLLOW2]-96)/32+vc_clip((int)p[VC_OSC3_TUNE]-64,-48,48);
        v->osc3_base_inc=440*exp2f((v->pitch-69+s->pitch_offset+v->unison_cents/100+offset+vc_detune3[p[VC_OSC3_DETUNE]]/100)/12)/s->sample_rate;
        v->osc3_gain=1.673f*p[VC_OSC3_VOLUME]/127;
      }
      if(p[VC_SAT_DRIVE]<64)v->osc3_gain*=vc_oscVolume[p[VC_SAT_DRIVE]];
      v->inc3=fminf(.45f,v->osc3_base_inc);
    }
 }
static VC_HOT void vc_glide_control(VCSynth *s,VCVoice *v){
 float before=v->pitch;
#ifdef VC_MATRIX
 float portamento=v->portamento_value,coefficient=v->portamento_coef;
#else
 float portamento=s->param[VC_PORTAMENTO],coefficient=s->glide_coef;
#endif
 if(v->generated_glide_left){
  v->generated_glide_left--;
  v->pitch=v->generated_glide_left?v->pitch+v->generated_glide_delta:v->note;
 }else if(!portamento||fabsf(v->pitch-v->note)<.001f){v->pitch=v->note;v->glide_error=0;}
 else {float delta=(v->note-v->pitch)*coefficient-v->glide_error;float next=v->pitch+delta;v->glide_error=(next-v->pitch)-delta;v->pitch=next;}
 if(v->pitch==before)return;
 {void (*volatile pitch_bases)(VCSynth*,VCVoice*)=vc_pitch_bases;pitch_bases(s,v);}
 vc_pitch_band(v,0,v->inc[0]);vc_pitch_band(v,1,v->inc[1]);
 /* Indirect call crosses the target SRAM/XIP branch-distance boundary. */
 void (*volatile setup)(VCSaturation*,float,unsigned,float,float)=vc_sat_setup;
 setup(&v->saturation,s->sample_rate,s->param[VC_SAT_CURVE],s->param[VC_SAT_DRIVE],v->pitch+s->pitch_offset+v->unison_cents/100-60);
}
#ifdef VC_MATRIX
static VC_HOT void vc_unison_control(VCSynth *s,VCVoice *v,float delta){
 if(v->unison_side==0)return;
 float amount=vc_clip(s->param[VC_UNISON_DETUNE]+delta,0,127);
 unsigned i=(unsigned)amount;if(i>126)i=126;
 float detune=vc_unison_detune[i]+(amount-i)*(vc_unison_detune[i+1]-vc_unison_detune[i]);
 float cents=v->unison_side*detune+(v->unison_odd?2.37f:0);
 if(cents==v->unison_cents)return;
 v->unison_cents=cents;{void (*volatile pitch_bases)(VCSynth*,VCVoice*)=vc_pitch_bases;pitch_bases(s,v);}
 vc_pitch_band(v,0,v->inc[0]);vc_pitch_band(v,1,v->inc[1]);
 void (*volatile setup)(VCSaturation*,float,unsigned,float,float)=vc_sat_setup;
 setup(&v->saturation,s->sample_rate,s->param[VC_SAT_CURVE],s->param[VC_SAT_DRIVE],v->pitch+s->pitch_offset+cents/100-60);
}
#endif
static void vc_amp_update(VCSynth *s,VCVoice *v);
#ifdef VC_MATRIX
/* Original ADSR matrix targets. Coefficients are recalculated only when their
 * effective parameter changes; phase/stage/level are never reset. This setup
 * code remains in flash, outside the constrained executable-RAM sample path. */
static const uint8_t vc_envelope_ids[10]={VC_F_ATTACK,VC_F_DECAY,VC_F_SUSTAIN,VC_F_SLOPE,VC_F_RELEASE,VC_ATTACK,VC_DECAY,VC_SUSTAIN,VC_AMP_SLOPE,VC_RELEASE};
static float vc_envelope_time(float value){
 unsigned i=(unsigned)value;if(i>=127)return vc_decay[127];
 if(value==i)return vc_decay[i];
 float a=1/vc_decay[i],b=1/vc_decay[i+1];
 return 1/(a+(value-i)*(b-a));
}
static void vc_lfo_matrix_control(VCSynth *s,VCVoice *v,const float *delta){
 const uint8_t *p=s->param;
 static const unsigned rate_ids[3]={VC_LFO_RATE,VC_LFO2_RATE,VC_LFO3_RATE};
 static const unsigned key_ids[3]={VC_LFO_KEYTRACK,VC_LFO2_KEYTRACK,VC_LFO3_KEYTRACK};
 static const unsigned clock_ids[3]={VC_LFO_CLOCK,VC_LFO2_CLOCK,VC_LFO3_CLOCK};
 VCLfo *lfos[3]={&v->lfo,&v->lfo2,&v->lfo3};
 const float deviation[3]={v->lfo_deviation,v->lfo2_deviation,v->lfo3_deviation};
 for(unsigned i=0;i<3;i++){
  float value=vc_clip(p[rate_ids[i]]+(delta?delta[i*2]:0),0,127);
  if(value!=v->lfo_rate_values[i]){
   v->lfo_rate_values[i]=value;
   float base;
   if(value==(unsigned)value)base=(i==2?vc_lfo3Rate:vc_lfoRate)[(unsigned)value];
   else {base=vc_lookup(vc_lfoRate,value);if(i==2&&value<127)base*=1.0058f;}
   lfos[i]->rate=vc_lfo_rate(base,v->note,p[key_ids[i]],deviation[i],p[clock_ids[i]],p[VC_TEMPO]);
  }
  if(i<2)lfos[i]->contour=vc_clip(p[i?VC_LFO2_CONTOUR:VC_LFO_CONTOUR]+(delta?delta[i*2+1]:0),0,127)-64;
 }
}
static __attribute__((noinline)) void vc_envelope_control(VCSynth *s,VCVoice *v,const float *delta){
 for(unsigned k=0;k<10;k++){
  unsigned field=k%5;float value=vc_clip(s->param[vc_envelope_ids[k]]+(delta?delta[k]:0),0,127);
  if(field==3)value=(unsigned)(value+.5f);
  if(value==v->envelope_values[k])continue;
  v->envelope_values[k]=value;VCEnvelope *e=k<5?&v->filter_env:&v->env;
  if(field==0){float time=k<5?2*vc_envelope_time(value):vc_lookup(vc_attack,value);e->attack_inc=1/fmaxf(1,time*s->sample_rate);}
  else if(field==1)e->decay_coef=expf(-1/fmaxf(1,vc_envelope_time(value)*s->sample_rate));
  else if(field==2)e->sustain=k<5?value/127:vc_lookup(vc_sustain,value);
  else if(field==3)vc_env_slope(e,s->sample_rate,(unsigned)value);
  else e->release_coef=expf(-1/fmaxf(1,vc_envelope_time(value)*s->sample_rate));
 }
 vc_lfo_matrix_control(s,v,delta?delta+10:0);
 float value=vc_clip(s->param[VC_PORTAMENTO]+(delta?delta[15]:0),0,127);
 if(value!=v->portamento_value){
  v->portamento_value=value;
  if(value==0)v->portamento_coef=1;
  else if(value==(unsigned)value&&s->sample_rate==44100)v->portamento_coef=vc_glideCoef[(unsigned)value];
  else {
   float time=.002f*exp2f(value*(11.287712379549449f/127));
   float x=VC_CONTROL_FRAMES/(time*s->sample_rate);
   v->portamento_coef=x<.01f?x*(1-x*(.5f-x*(1.f/6-x/24))):1-expf(-x);
  }
 }

 vc_noise_setup(&v->noise,s->sample_rate,(unsigned)(vc_clip(s->param[VC_NOISE_COLOR]+(delta?delta[19]:0),0,127)+.5f));
 v->osc2_matrix_detune=vc_lookup(vc_detune,vc_clip(s->param[VC_DETUNE]+(delta?delta[20]:0),0,127))-vc_detune[s->param[VC_DETUNE]];
 float key=v->pitch+s->pitch_offset+v->unison_cents/100-60;
 v->transpose_matrix=delta?delta[28]:0;
 for(unsigned i=0;i<2;i++){
  unsigned id=i?VC_KEYFOLLOW2:VC_KEYFOLLOW1;
  float effective=vc_clip(s->param[id]+(delta?delta[21+i]:0),0,127);
  v->key_matrix_cents[i]=(key*(effective-s->param[id])+v->transpose_matrix*(effective-64))*(100.f/32);
 }
 vc_source_levels(s,v,delta?delta+23:0);
 v->osc3_matrix_cents=0;
 if(s->param[VC_OSC3_MODE]>1){
  const uint8_t *p=s->param;
  float volume=vc_clip(p[VC_OSC3_VOLUME]+(delta?delta[16]:0),0,127);
  float tune=vc_clip(p[VC_OSC3_TUNE]+(delta?delta[17]:0),0,127);
  float detune=vc_clip(p[VC_OSC3_DETUNE]+(delta?delta[18]:0),0,127);
  unsigned i=(unsigned)detune;if(i>126)i=126;
  float cents=vc_detune3[i]+(detune-i)*(vc_detune3[i+1]-vc_detune3[i]);
  v->osc3_matrix_cents=100*(vc_clip((int)(tune+.5f)-64,-48,48)-vc_clip((int)p[VC_OSC3_TUNE]-64,-48,48))+cents-vc_detune3[p[VC_OSC3_DETUNE]];
  v->osc3_gain=1.673f*volume/127;
  v->osc3_gain*=v->source_gain[5];
 }else if(s->param[VC_OSC3_MODE]==1)v->osc3_gain=1.058f*v->source_gain[4]*v->source_gain[5];
}
void vc_set_matrix(VCSynth *s,const VCMatrix *m){
 s->matrix=vc_matrix_has_filters(m)?m:0;
 s->matrix_source_count=0;
 unsigned seen=0;
 if(s->matrix)for(unsigned i=0;i<VC_MATRIX_ROUTES;i++){
  unsigned source=m->route[i].source;
  if(!source||source>=VC_MATRIX_SOURCES||(seen&(1u<<source)))continue;
  seen|=1u<<source;
  s->matrix_source_ids[s->matrix_source_count++]=(uint8_t)source;
 }
 for(unsigned i=0;i<VC_VOICES;i++){
  VCVoice *v=&s->voice[i];
  v->control_remaining=0;v->matrix_delta_valid=0;
  if(v->env.stage){vc_amp_update(s,v);vc_envelope_control(s,v,0);}
 }
}
void vc_pressure(VCSynth *s,int value){s->controllers[2]=vc_clip(value,0,127)/127.f;}
static __attribute__((noinline)) void vc_matrix_controls(const VCMatrix *m,const float *sources,float *delta,const uint8_t *p){
 vc_matrix_filters(m,sources,delta);
 /* Keep discrete rounding beside the matrix evaluator in flash. */
 static const uint8_t indices[4]={47,52,61,63},ids[4]={VC_F_SLOPE,VC_AMP_SLOPE,VC_OSC3_TUNE,VC_NOISE_COLOR},destinations[4]={36,41,98,91};
 for(unsigned k=0;k<4;k++){
  unsigned index=indices[k],id=ids[k];
  float value=vc_clip(p[id]+delta[index],0,127);
  if(value-(unsigned)value==.5f)delta[index]=(float)vc_matrix_rounded(m,sources,destinations[k],p[id])-p[id];
 }
}
static inline float vc_matrix_source_value(const VCSynth *s,const VCVoice *v,unsigned id){
 switch(id){
 case 1:return s->bend;
 case 19:return v->env.level;
 case 20:return v->filter_env.level;
 case 21:return v->lfo_mod;
 case 22:return v->lfo2_mod;
 case 23:return v->lfo3_mod;
 case 24:return v->velocity;
 case 25:return v->release_velocity;
 case 26:return vc_clip((v->note-60)/64.f,-1,1);
 default:return s->controllers[id];
 }
}
static __attribute__((noinline)) const float *vc_matrix_filter_delta(VCSynth *s,VCVoice *v){
 /* Only routed sources can change the result, including route-amount sources.
  * Parameter or matrix changes invalidate the cached coefficients. */
 float sources[VC_MATRIX_SOURCES];
 unsigned unchanged=v->matrix_delta_valid;
 for(unsigned i=0;i<s->matrix_source_count;i++){
  unsigned id=s->matrix_source_ids[i];
  float value=vc_matrix_source_value(s,v,id);
  sources[id]=value;
  if(v->matrix_sources[i]!=value){v->matrix_sources[i]=value;unchanged=0;}
 }
 if(unchanged)return v->matrix_delta;
 vc_matrix_controls(s->matrix,sources,v->matrix_delta,s->param);
#ifdef VC_BAND_CACHE
 vc_band_wave_control(s->matrix,sources,s->param,v->osc);
#endif
 v->matrix_delta_valid=1;
 return v->matrix_delta;
}
/* Original k curve: promote 127 to 128, then interpolate magnitude anchors. */
static inline __attribute__((always_inline)) float vc_lfo_pitch_float(float value){
 static const float points[9]={0,9.5f,31.5f,81,200,400,800,1600,3200};
 float x=vc_clip(value,0,127);if(x>=127)x=128;x-=64;
 float at=fabsf(x)/8;unsigned i=(unsigned)at;if(i>7)i=7;
 float cents=points[i]+(at-i)*(points[i+1]-points[i]);return x<0?-cents:cents;
}
static inline __attribute__((always_inline)) float vc_lfo_depth_float(float value){
 float x=vc_clip(value,0,127);if(x>=127)x=128;x-=64;
 float n=fabsf(x),r=n<=16?n*.25f:n<=32?4+(n-16)*.75f:n<=48?16+(n-32):32+2*(n-48);
 return x<0?-r:r;
}
static inline __attribute__((always_inline)) float vc_linked_cutoff_float(float value){
 float x=vc_clip(value,0,127)-64,n=fabsf(x);
 float r=n<=16?.5f*n:n<=32?8+1.5f*(n-16):32+2*(n-32);
 return x<0?-r:r;
}
#endif
/* Source Ae: 16-unit anchors, with 127 promoted to 128 before interpolation. */
static inline __attribute__((always_inline)) float vc_velocity_env_amount(float x){
 static const float points[9]={0,8,16,32,48,68,88,108,128};
 x=vc_clip(x,0,127);if(x>=127)x=128;float at=x/16;unsigned i=(unsigned)at;if(i>7)i=7;
 return points[i]+(at-i)*(points[i+1]-points[i]);
}
/* Source T amplifier velocity curve; 128 preserves the native linear response. */
static inline __attribute__((always_inline)) float vc_amp_velocity(float amount,float velocity){
 if(amount>=128)return velocity;
 float depth=amount-64;if(!depth)return 1;
 float midi=velocity*127;
 float db=depth>0?-.1009f*depth*((127-midi)/32):-.3338f*(-depth)*(midi/95);
 return exp2f(db*.1660964047443681f);
}
static inline __attribute__((always_inline)) void vc_amp_set_target(VCVoice *v,float target){
 if(!v->env.stage){v->amp_gain=target;v->amp_remaining=0;}
 else if(target!=v->amp_target){v->amp_step=(target-v->amp_gain)/VC_CONTROL_FRAMES;v->amp_remaining=VC_CONTROL_FRAMES;}
 v->amp_target=target;
}
/* Filter coefficients use the source's 32-frame control cadence. The cache
 * skips static configurations; the reciprocal is calculated here, not per sample. */
static VC_HOT __attribute__((noinline)) void vc_filter_control_at(VCSynth *s,VCVoice *v,const float global[3]){
    vc_glide_control(s,v);
    v->lfo_mod=s->param[VC_LFO_MODE]?(s->param[VC_LFO_GLOBAL]?global[0]:v->lfo.value):0;
    v->lfo2_mod=s->param[VC_LFO2_MODE]?(s->param[VC_LFO2_GLOBAL]?global[1]:v->lfo2.value):0;
    if(v->lfo3_elapsed_frames<(uint32_t)(s->sample_rate*64))v->lfo3_elapsed_frames+=VC_CONTROL_FRAMES;
    v->lfo3_mod=0;
    if(s->param[VC_LFO3_ENABLE]){
      float time=vc_lfo3Fade[s->param[VC_LFO3_FADE]],fade=1;
      if(time>0)fade=vc_clip((v->lfo3_elapsed_frames/s->sample_rate-time/9)/(time-time/9),0,1);
      v->lfo3_mod=(s->param[VC_LFO3_GLOBAL]?global[2]:v->lfo3.value)*fade;
    }
#ifndef VC_MATRIX
    vc_pitch_env_control(s,v);
#endif
    const uint8_t *p=s->param;
    float lfo_cutoff[2]={vc_lfoResonance[p[VC_LFO2_CUTOFF1]],vc_lfoResonance[p[VC_LFO2_CUTOFF2]]};
    float resonance=vc_lfoResonance[p[VC_LFO_RESONANCE]]*v->lfo_mod;
    v->lfo_filter_gain=fmaxf(0,1+vc_lfoFilterGain[p[VC_LFO_FILTER_GAIN]]*v->lfo_mod);
    float key=v->pitch+s->pitch_offset+v->unison_cents/100-p[VC_F_KEYBASE];
    float filter_value[4]={p[VC_CUTOFF],p[VC_CUTOFF2],p[VC_RESONANCE],p[VC_RESONANCE2]};
#ifdef VC_MATRIX
    const float *delta=0;
    if(s->matrix){
      const float *(*volatile evaluate)(VCSynth*,VCVoice*)=vc_matrix_filter_delta;
      delta=evaluate(s,v);
      for(unsigned k=0;k<4;k++)filter_value[k]+=delta[k];
    }
    vc_unison_control(s,v,delta?delta[42]:0);
    key=v->pitch+s->pitch_offset+v->unison_cents/100-p[VC_F_KEYBASE]+(delta?delta[72]:0);
    if(!s->matrix)vc_pitch_env_control(s,v);
    if(s->matrix){
      void (*volatile envelopes)(VCSynth*,VCVoice*,const float*)=vc_envelope_control;envelopes(s,v,delta+44);
      float amp_amount=delta[41]==0?p[VC_AMP_VELOCITY]:vc_clip(p[VC_AMP_VELOCITY]+delta[41],0,127);
      if(amp_amount!=v->amp_velocity_amount){v->amp_velocity_amount=amp_amount;v->amp_velocity_gain=vc_amp_velocity(amp_amount,v->velocity);}
      float volume=vc_clip(p[VC_PATCH_VOLUME]+delta[39],0,127);unsigned vi=(unsigned)volume;if(vi>126)vi=126;
      float patch_gain=vc_patchVolume[vi]+(volume-vi)*(vc_patchVolume[vi+1]-vc_patchVolume[vi]);
      vc_amp_set_target(v,v->amp_velocity_gain*s->channel_volume*s->expression*patch_gain*vc_clip(1+delta[40]/127,0,2));
      unsigned dest=p[VC_LFO3_DEST];
      float lfo3_amount=vc_clip(p[VC_LFO3_AMOUNT]+delta[43],0,127);
      unsigned lfo3_index=(unsigned)lfo3_amount;if(lfo3_index>126)lfo3_index=126;
      float pitch3=(vc_lfo3Pitch[lfo3_index]+(lfo3_amount-lfo3_index)*(vc_lfo3Pitch[lfo3_index+1]-vc_lfo3Pitch[lfo3_index]))*v->lfo3_mod;
      float cents1=vc_lfo_pitch_float(p[VC_LFO_OSC1]+delta[37])*v->lfo_mod+(dest<=1?pitch3:0)+100*vc_clip(delta[34],-48,36)+v->key_matrix_cents[0];
      float cents2=2*vc_lfo_pitch_float(p[VC_OSC2_ENV]+delta[36])*v->filter_env.level
        +vc_lfo_pitch_float(p[VC_LFO_OSC2]+delta[38])*v->lfo_mod+(dest==1||dest==2?pitch3:0)+100*vc_clip(delta[35],-48,36)+v->osc2_matrix_detune+v->key_matrix_cents[1];
      vc_pitch_mod_control(v,cents1,cents2);
      float pwm=2*(vc_clip(p[VC_VEL_PW]+delta[33],0,127)-64)*v->velocity
        +(vc_clip(p[VC_LFO_PW]+delta[32],0,127)-64)*(127.f/64)*v->lfo_mod;
      vc_pwm_control(s,v,pwm,delta[30],delta[31],lfo3_amount);
      float shape_lfo=(vc_clip(p[VC_LFO2_SHAPE_AMOUNT]+delta[27],0,127)-64)*(127.f/128)*v->lfo2_mod;
      float old_wave=v->osc[0].wave_gain,old_saw=v->osc[0].saw_gain;
      for(unsigned i=0;i<2;i++){
        float velocity=2*(vc_clip(p[i?VC_VEL_SHAPE2:VC_VEL_SHAPE1]+delta[28+i],0,127)-64)*v->velocity;
        vc_osc_shape(&v->osc[i],vc_clip(p[i?VC_SHAPE2:VC_SHAPE1]+delta[25+i]+velocity+shape_lfo,0,127));
      }
      if(old_wave!=v->osc[0].wave_gain||old_saw!=v->osc[0].saw_gain)
        v->fm_scale=1/fmaxf(.1f,v->osc[0].wave_gain+v->osc[0].saw_gain/.669f);
      /* Force the normal shape path to restore cached base values if cleared. */
      v->lfo2_shape_last=1e30f;
      v->fm_depth=vc_fm_depth_curve(p[VC_FM_AMOUNT]+delta[21]
        +2*(vc_clip(p[VC_FM_ENV]+delta[22],0,127)-64)*v->filter_env.level
        +2*(vc_clip(p[VC_LFO2_FM_AMOUNT]+delta[23],0,127)-64)*v->lfo2_mod
        +2*(vc_clip(p[VC_FM_VELOCITY]+delta[24],0,127)-64)*v->velocity);
      vc_filter_mix(p[VC_FILTER_BALANCE]+delta[16],p[VC_ROUTING],v->filter_mix,&v->filter_mix_after);
      resonance=vc_lfo_depth_float(p[VC_LFO_RESONANCE]+delta[12])*v->lfo_mod;
      float gain=vc_clip(p[VC_LFO_FILTER_GAIN]+delta[13],0,127)-64;
      v->lfo_filter_gain=fmaxf(0,1+gain/(gain>=0?63.f:64.f)*v->lfo_mod);
      lfo_cutoff[0]=vc_lfo_depth_float(p[VC_LFO2_CUTOFF1]+delta[14]);
      lfo_cutoff[1]=vc_lfo_depth_float(p[VC_LFO2_CUTOFF2]+delta[15]);
    }
    if(!s->matrix){vc_lfo2_osc_control(s,v);vc_fm_control(s,v);}
    float c2=p[VC_CUTOFF_LINK]?filter_value[0]+vc_linked_cutoff_float(filter_value[1]):filter_value[1];
#else
    vc_lfo2_osc_control(s,v);vc_fm_control(s,v);
    float c2=p[VC_CUTOFF_LINK]?p[VC_CUTOFF]+vc_linkedCutoff[p[VC_CUTOFF2]]:p[VC_CUTOFF2];
#endif
#ifdef VC_DUAL_RENDER
    void (*volatile osc3_control)(VCSynth*,VCVoice*)=vc_osc3_control;
    osc3_control(s,v);
#else
    vc_osc3_control(s,v);
#endif
#ifdef VC_MATRIX
    if(s->matrix){
      float pan=p[VC_PAN]+delta[17]+2*(vc_clip(p[VC_VEL_PAN]+delta[20],0,127)-64)*v->velocity
        +2*(vc_clip(p[VC_LFO2_PAN]+delta[18],0,127)-64)*v->lfo2_mod;
      float spread=vc_clip(p[VC_SPREAD]+delta[19],0,127);
      unsigned enabled=(p[VC_LFO2_MODE]&&p[VC_LFO2_PAN]!=64)||p[VC_VEL_PAN]!=64
        ||delta[17]!=0||delta[18]!=0||delta[19]!=0||delta[20]!=0;
      vc_pan_control_values(s,v,pan,spread,enabled);
    }else
#endif
    vc_lfo2_pan_control(s,v);
    for(int i=0;i<2;i++){
        float follow=p[i?VC_F_KEYFOLLOW2:VC_F_KEYFOLLOW1];
        float env_amount=v->filter_env_amount[i],res_velocity=v->filter_res_velocity[i];
#ifdef VC_MATRIX
        if(s->matrix){
          follow=vc_clip(follow+delta[6+i],0,127);
          float vel_env=vc_clip(p[i?VC_VEL_ENV2:VC_VEL_ENV1]+delta[8+i],0,127);
          env_amount=vc_velocity_env_amount(p[i?VC_F_AMOUNT2:VC_F_AMOUNT1]+delta[4+i]+2*(vel_env-64)*v->velocity);
          res_velocity=2*(vc_clip(p[i?VC_VEL_RES2:VC_VEL_RES1]+delta[10+i],0,127)-64)*v->velocity;
        }
#endif
        if(follow>=127)follow=128;
        float env=env_amount*v->filter_env.level;
        if(!p[i?VC_F_POLARITY2:VC_F_POLARITY1])env=-env;
        float cutoff=(i?c2:filter_value[0])+env+.01676f*(follow-64)*key;
        cutoff+=lfo_cutoff[i]*v->lfo2_mod; /* source k curve */
        vc_filter_setup(&v->filter[i],s->sample_rate,p[i?VC_FILTER_MODE2:VC_FILTER_MODE],
            !i&&p[VC_ROUTING]==1?4:2,vc_clip(cutoff,0,128),vc_clip(filter_value[2+i]+res_velocity+resonance,0,127));
    }
    /* Capture source values first, apply matrix timing, then advance local
     * LFOs once. Shared LFOs keep the original independent base timing. */
    if(p[VC_LFO_MODE])vc_lfo_tick(&v->lfo,VC_CONTROL_FRAMES);
    if(p[VC_LFO2_MODE])vc_lfo_tick(&v->lfo2,VC_CONTROL_FRAMES);
    if(p[VC_LFO3_ENABLE])vc_lfo_tick(&v->lfo3,VC_CONTROL_FRAMES);
}
static inline __attribute__((always_inline)) void vc_filter_control(VCSynth *s,VCVoice *v){
 const float global[]={s->global_lfo[0].value,s->global_lfo[1].value,s->global_lfo[2].value};
 void (*volatile control)(VCSynth*,VCVoice*,const float*)=vc_filter_control_at;
 control(s,v,global);
}
static inline __attribute__((always_inline)) float vc_amp_gain_next(VCVoice *v){
 if(v->amp_remaining){v->amp_gain+=v->amp_step;if(!--v->amp_remaining)v->amp_gain=v->amp_target;}
 return v->amp_gain;
}
/* Controller changes only update cached amplitude, preserving oscillator and
 * modulation state. Existing native 32-sample smoothing applies. */
static void vc_amp_update(VCSynth *s,VCVoice *v){
    float velocity_gain=vc_amp_velocity(s->param[VC_AMP_VELOCITY],v->velocity);
#ifdef VC_MATRIX
    v->amp_velocity_amount=s->param[VC_AMP_VELOCITY];v->amp_velocity_gain=velocity_gain;
#endif
    vc_amp_set_target(v,velocity_gain*s->channel_volume*s->expression*vc_patchVolume[s->param[VC_PATCH_VOLUME]]);
}
static void vc_update_voice(VCSynth *s,VCVoice *v){
 vc_source_levels(s,v,0);
 v->unison_cents=v->unison_side==0?0:v->unison_side*vc_unison_detune[s->param[VC_UNISON_DETUNE]]+(v->unison_odd?2.37f:0);
    float bend=s->bend;
    if(s->param[VC_BEND_SCALE])bend*=fabsf(bend);
    int range=s->bend>=0?(int)s->param[VC_BEND_UP]-64:64-(int)s->param[VC_BEND_DOWN];
    s->pitch_offset=(int)s->param[VC_TRANSPOSE]-64+bend*range;
    const uint8_t *p=s->param;
    for(unsigned i=0;i<2;i++){
      unsigned amount=p[i?VC_F_AMOUNT2:VC_F_AMOUNT1];
      int env_velocity=(int)p[i?VC_VEL_ENV2:VC_VEL_ENV1]-64;
      v->filter_env_amount[i]=env_velocity?vc_velocity_env_amount(amount+2*env_velocity*v->velocity):vc_envAmount[amount];
      v->filter_res_velocity[i]=2*((int)p[i?VC_VEL_RES2:VC_VEL_RES1]-64)*v->velocity;
    }
    v->velocity_shape[0]=2*((int)p[VC_VEL_SHAPE1]-64)*v->velocity;
    v->velocity_shape[1]=2*((int)p[VC_VEL_SHAPE2]-64)*v->velocity;
    v->velocity_pw=2*((int)p[VC_VEL_PW]-64)*v->velocity;
    v->velocity_pan=2*((int)p[VC_VEL_PAN]-64)*v->velocity;
    vc_amp_update(s,v);
    v->punch_gain=p[VC_PUNCH]?vc_punchLevel[p[VC_PUNCH]]*vc_oscVolume[p[VC_SAT_DRIVE]]:0;
    if(!v->generated_glide_left&&!p[VC_PORTAMENTO]
#ifdef VC_MATRIX
       && !(s->matrix&&v->env.stage&&v->portamento_value>0)
#endif
    ){v->pitch=v->note;v->glide_error=0;}
    vc_sat_setup(&v->saturation,s->sample_rate,p[VC_SAT_CURVE],p[VC_SAT_DRIVE],v->pitch+s->pitch_offset+v->unison_cents/100-60);
    s->glide_coef=s->sample_rate==44100?vc_glideCoef[p[VC_PORTAMENTO]]:p[VC_PORTAMENTO]?1-expf(-VC_CONTROL_FRAMES/(vc_portamento[p[VC_PORTAMENTO]]*s->sample_rate)):1;
#ifdef VC_MATRIX
    v->portamento_value=p[VC_PORTAMENTO];v->portamento_coef=s->glide_coef;
#endif
    v->lfo2_shape_last=0;v->pw_last[0]=v->pw_last[1]=0;
    void (*volatile pitch_bases)(VCSynth*,VCVoice*)=vc_pitch_bases;pitch_bases(s,v);
    vc_osc_setup(&v->osc[0],p[VC_WAVE1],vc_clip(p[VC_SHAPE1]+v->velocity_shape[0],0,127),.5f*(1-vc_clip(p[VC_PW1]+v->velocity_pw,-63.5f,127)/127.f),v->inc[0]);
    vc_osc_setup(&v->osc[1],p[VC_WAVE2],vc_clip(p[VC_SHAPE2]+v->velocity_shape[1],0,127),.5f*(1-vc_clip(p[VC_PW2]+v->velocity_pw,-63.5f,127)/127.f),v->inc[1]);
    if(p[VC_OSC3_MODE]>=4){
#if defined(VC_BAND_CACHE)
      v->osc3.tables=0;v->osc3.table=vc_band_silence;
#elif defined(VC_DYNAMIC_WAVES)
      v->osc3.tables=vc_wave_tables(p[VC_OSC3_MODE]-4);
#else
      v->osc3.tables=vc_wave_views[p[VC_OSC3_MODE]-4];
#endif
    }
    vc_env_setup(&v->env,s->sample_rate,vc_attack[p[VC_ATTACK]],vc_decay[p[VC_DECAY]],vc_sustain[p[VC_SUSTAIN]],vc_release[p[VC_RELEASE]]);
    vc_env_slope(&v->env,s->sample_rate,p[VC_AMP_SLOPE]);
    vc_env_setup(&v->filter_env,s->sample_rate,vc_filterAttack[p[VC_F_ATTACK]],vc_decay[p[VC_F_DECAY]],p[VC_F_SUSTAIN]/127.f,vc_release[p[VC_F_RELEASE]]);
    vc_env_slope(&v->filter_env,s->sample_rate,p[VC_F_SLOPE]);
#ifdef VC_MATRIX
    v->matrix_delta_valid=0;
    for(unsigned k=0;k<10;k++)v->envelope_values[k]=p[vc_envelope_ids[k]];
    v->lfo_rate_values[0]=p[VC_LFO_RATE];v->lfo_rate_values[1]=p[VC_LFO2_RATE];v->lfo_rate_values[2]=p[VC_LFO3_RATE];
#endif
    vc_noise_setup(&v->noise,s->sample_rate,p[VC_NOISE_COLOR]);
    v->fm_scale=1/fmaxf(.1f,v->osc[0].wave_gain+v->osc[0].saw_gain/.669f);
    vc_fm_apply(s,v);
    vc_lfo_config(&v->lfo,vc_lfo_rate(vc_lfoRate[p[VC_LFO_RATE]],v->note,p[VC_LFO_KEYTRACK],v->lfo_deviation,p[VC_LFO_CLOCK],p[VC_TEMPO]),p[VC_LFO_SHAPE],(int)p[VC_LFO_CONTOUR]-64,p[VC_LFO_MODE]==2);
    if(!p[VC_LFO_MODE])v->lfo_mod=0;
    vc_lfo_config(&v->lfo2,vc_lfo_rate(vc_lfoRate[p[VC_LFO2_RATE]],v->note,p[VC_LFO2_KEYTRACK],v->lfo2_deviation,p[VC_LFO2_CLOCK],p[VC_TEMPO]),p[VC_LFO2_SHAPE],(int)p[VC_LFO2_CONTOUR]-64,p[VC_LFO2_MODE]==2);
    if(!p[VC_LFO2_MODE])v->lfo2_mod=0;
    vc_lfo_config(&v->lfo3,vc_lfo_rate(vc_lfo3Rate[p[VC_LFO3_RATE]],v->note,p[VC_LFO3_KEYTRACK],v->lfo3_deviation,p[VC_LFO3_CLOCK],p[VC_TEMPO]),p[VC_LFO3_SHAPE],0,0);
    if(!p[VC_LFO3_ENABLE])v->lfo3_mod=0;
    v->lfo_filter_gain=fmaxf(0,1+vc_lfoFilterGain[p[VC_LFO_FILTER_GAIN]]*v->lfo_mod);
    v->control_remaining=0;
}
/* Shared source LFOs run independently of key tracking and voice deviation.
 * Keep them running while idle/per-voice so switching does not reset phase. */
static void vc_global_config(VCSynth *s){
 const uint8_t *p=s->param;
 for(unsigned i=0;i<2;i++)vc_lfo_config(&s->global_lfo[i],
  vc_lfo_rate(vc_lfoRate[p[i?VC_LFO2_RATE:VC_LFO_RATE]],60,0,1,
   p[i?VC_LFO2_CLOCK:VC_LFO_CLOCK],p[VC_TEMPO]),
  p[i?VC_LFO2_SHAPE:VC_LFO_SHAPE],(int)p[i?VC_LFO2_CONTOUR:VC_LFO_CONTOUR]-64,
  p[i?VC_LFO2_MODE:VC_LFO_MODE]==2);
 vc_lfo_config(&s->global_lfo[2],vc_lfo_rate(vc_lfo3Rate[p[VC_LFO3_RATE]],60,0,1,p[VC_LFO3_CLOCK],p[VC_TEMPO]),p[VC_LFO3_SHAPE],0,0);
}
void vc_init(VCSynth *s,float sr){
    memset(s,0,sizeof(*s));s->sample_rate=sr;s->last_note=-1;s->glide_coef=1;s->channel_volume=s->expression=1;
    for(unsigned i=0;i<VC_VOICES;i++)s->voice[i].lfo_filter_gain=1;
    uint8_t p[VC_PARAMS]={64,64,64,20,100,20,0,0,8,45,100,40,0,0,0,127,
        64,0,20,0,60,0,40,0,0,1,1,0,64,64,60,64,64,64,64};
    memcpy(s->param,p,sizeof(p));s->param[VC_NOISE_COLOR]=64;
    s->param[VC_TUNE1]=s->param[VC_TUNE2]=64;
    s->param[VC_KEYFOLLOW1]=s->param[VC_KEYFOLLOW2]=96;
    for(unsigned id=VC_EQ_LOW_GAIN;id<=VC_EQ_HIGH_FREQ;id++)s->param[id]=64;
    s->param[VC_BOOST_TUNE]=64;s->param[VC_UNISON_DETUNE]=48;s->param[VC_UNISON_PHASE]=64;
    s->param[VC_TEMPO]=57; /* source mapping: 63 + raw = 120 BPM */
    s->param[VC_SAT_DRIVE]=64;s->param[VC_PATCH_VOLUME]=90;
    s->param[VC_TRANSPOSE]=64;s->param[VC_BEND_UP]=66;s->param[VC_BEND_DOWN]=62;
    s->param[VC_AMP_VELOCITY]=128;
    s->param[VC_VEL_SHAPE1]=s->param[VC_VEL_SHAPE2]=s->param[VC_VEL_PW]=s->param[VC_VEL_PAN]=64;
    s->param[VC_VEL_ENV1]=s->param[VC_VEL_ENV2]=s->param[VC_VEL_RES1]=s->param[VC_VEL_RES2]=64;
    s->param[VC_OSC3_VOLUME]=64;s->param[VC_OSC3_TUNE]=s->param[VC_OSC3_DETUNE]=64;
    s->param[VC_LFO3_RATE]=64;s->param[VC_LFO3_DEST]=1;
    s->param[VC_LFO2_PAN]=64;
    s->param[VC_LFO2_SHAPE_AMOUNT]=s->param[VC_LFO2_FM_AMOUNT]=64;
    s->param[VC_LFO2_RATE]=64;s->param[VC_LFO2_CONTOUR]=64;s->param[VC_LFO2_TRIGGER]=1;
    s->param[VC_LFO2_CUTOFF1]=s->param[VC_LFO2_CUTOFF2]=64;
    s->param[VC_LFO_RATE]=64;s->param[VC_LFO_CONTOUR]=64;s->param[VC_LFO_TRIGGER]=1;
    s->param[VC_LFO_OSC1]=s->param[VC_LFO_OSC2]=s->param[VC_LFO_PW]=64;
    s->param[VC_LFO_RESONANCE]=s->param[VC_LFO_FILTER_GAIN]=64;
    s->param[VC_INIT_PHASE]=128; /* compatibility: both oscillators start at zero */
    s->param[VC_FM_ENV]=s->param[VC_FM_VELOCITY]=s->param[VC_OSC2_ENV]=64;vc_route_setup(s);
    vc_lfo_init(&s->global_lfo[0],sr,0x1279ac34u);
    vc_lfo_init(&s->global_lfo[1],sr,0x739b2df1u);vc_lfo_init(&s->global_lfo[2],sr,0x5239b75du);vc_global_config(s);
}
unsigned vc_param_max(unsigned id){
    switch(id){
    case VC_EQ_ENABLE:return 1;
    case VC_DIST_MODE:return 11;
    case VC_UNISON_MODE:return 3;
    case VC_KEY_MODE:return 5;
    case VC_SAT_CURVE:return 14;
    case VC_INIT_PHASE:case VC_AMP_VELOCITY:return 128;
    case VC_LFO_CLOCK:case VC_LFO2_CLOCK:case VC_LFO3_CLOCK:return 21;
    case VC_LFO_SHAPE:case VC_LFO2_SHAPE:case VC_LFO3_SHAPE:return VC_LFO_MAX_SHAPE;
    case VC_LFO3_DEST:return 5;
    case VC_LFO3_GLOBAL:case VC_LFO_GLOBAL:case VC_LFO2_GLOBAL:case VC_LFO3_ENABLE:case VC_BEND_SCALE:return 1;
    case VC_LFO_MODE:case VC_LFO2_MODE:return 2;
    case VC_OSC3_MODE:return VC_WAVES+3;
    case VC_FILTER_MODE:case VC_FILTER_MODE2:case VC_ROUTING:case VC_FM_MODE:return 3;
    case VC_WAVE1:case VC_WAVE2:return VC_WAVES-1;
    case VC_SUB_SHAPE:case VC_SYNC:case VC_F_POLARITY1:case VC_F_POLARITY2:case VC_CUTOFF_LINK:return 1;
    default:return 127;
    }
}
void vc_param(VCSynth *s,unsigned id,unsigned value){
    if(id>=VC_PARAMS)return;
    unsigned max=vc_param_max(id);if(value>max)value=max;
    if(s->param[id]==value)return;
    s->param[id]=(uint8_t)value;
#ifdef VC_POST_FX
    if(id>=VC_EQ_ENABLE&&id<=VC_EQ_HIGH_FREQ){
     if(id==VC_EQ_ENABLE)vc_eq_reset(&s->eq);
     vc_eq_set(&s->eq,s->param[VC_EQ_LOW_GAIN],s->param[VC_EQ_LOW_FREQ],s->param[VC_EQ_MID_GAIN],s->param[VC_EQ_MID_FREQ],s->param[VC_EQ_MID_Q],s->param[VC_EQ_HIGH_GAIN],s->param[VC_EQ_HIGH_FREQ]);return;
    }
    if(id==VC_BOOST_LEVEL||id==VC_BOOST_TUNE){vc_boost_set(&s->boost,s->param[VC_BOOST_LEVEL],s->param[VC_BOOST_TUNE]);return;}
    if(id==VC_DIST_MODE||id==VC_DIST_LEVEL){vc_distortion_set(&s->distortion,s->param[VC_DIST_MODE],s->param[VC_DIST_LEVEL]);return;}
#endif
    vc_global_config(s);
    if(id==VC_KEY_MODE||id==VC_UNISON_MODE){vc_panic(s);return;}
    if(id==VC_PATCH_VOLUME){for(unsigned i=0;i<VC_VOICES;i++)if(s->voice[i].env.stage)vc_amp_update(s,&s->voice[i]);return;}
    if(id==VC_INIT_PHASE)return; /* only newly started voices change phase */
    vc_route_setup(s);
    for(int i=0;i<VC_VOICES;i++)if(s->voice[i].env.stage)vc_update_voice(s,&s->voice[i]);
}
static void vc_voice_release(VCVoice *v){v->env.stage=4;v->filter_env.stage=4;
#ifdef VC_MATRIX
 v->release_velocity=0;
#endif
}
/* Separate note-start PRNG: do not advance or change the audible noise stream. */
static float vc_start_random(uint32_t *state){
 uint32_t x=*state;if(!x)x=1;x^=x<<13;x^=x>>17;x^=x<<5;*state=x;
 return (x>>8)*(1.f/16777216.f);
}
static void vc_unison_trigger(VCVoice *v,VCLfo *l,unsigned phase){
 vc_lfo_trigger(l,phase);
 if(v->unison_lfo_offset){l->phase+=v->unison_lfo_offset;l->phase-=floorf(l->phase);if(phase)l->start=l->phase;}
}
#ifdef VC_DUAL_RENDER
#define VC_VOICE_FADE 64u
/* Fade from the last mixed voice sample after a hard start or steal.
 * The bounded mix ramp adds no per-sample work after it has finished. */
static void vc_voice_fade(VCSynth *s,unsigned i){
 s->voice_fade_from[i][0]=s->voice_last[i][0];
 s->voice_fade_from[i][1]=s->voice_last[i][1];
 s->voice_fade_left[i]=VC_VOICE_FADE;
}
#endif
#ifdef VC_DUAL_RENDER
/* Overload relief: fade out the quietest voice that is already releasing,
 * with the same 32-sample fade a steal uses. Held voices are never cut.
 * Returns 1 if a voice was shed. */
int vc_shed(VCSynth *s){
 int pick=-1;
 for(unsigned i=0;i<VC_VOICES;i++){VCVoice *v=&s->voice[i];
  if(v->env.stage==4&&(pick<0||v->env.level<s->voice[pick].env.level))pick=(int)i;}
 if(pick<0)return 0;
 vc_voice_fade(s,(unsigned)pick);
 s->voice[pick].env.stage=0;s->voice[pick].held=0;
 return 1;
}
#endif
static void vc_start_voice(VCSynth *s,VCVoice *v,int n,int vel,int previous,float side,unsigned odd,float offset,unsigned age){
#ifdef VC_DUAL_RENDER
    unsigned slot=(unsigned)(v-s->voice);
    if(vc_attack[s->param[VC_ATTACK]]*s->sample_rate<VC_VOICE_FADE)vc_voice_fade(s,slot);
#endif
    memset(v,0,sizeof(*v));v->note=n;v->pitch=s->param[VC_PORTAMENTO]&&previous>=0?previous:n;v->held=1;v->velocity=vc_clip(vel/127.f,0,1);v->age=age;v->unison_side=side;v->unison_odd=odd;v->unison_lfo_offset=offset;
#ifdef VC_MATRIX
    v->release_velocity=v->velocity;
#endif
    v->noise.rng=0x9e3779b9u^((uint32_t)n*0x85ebca6bu)^(v->age*0xc2b2ae35u)^(s->param[VC_UNISON_MODE]?(unsigned)(v-s->voice)*0x27d4eb2du:0);
    vc_lfo_init(&v->lfo,s->sample_rate,v->noise.rng^0x6d2b79f5u);
    v->lfo_deviation=vc_lfo_deviation(&v->lfo);
    vc_lfo_init(&v->lfo2,s->sample_rate,v->noise.rng^0x1b873593u);
    v->lfo2_deviation=vc_lfo_deviation(&v->lfo2);
    vc_lfo_init(&v->lfo3,s->sample_rate,v->noise.rng^0x27d4eb2fu);
    v->lfo3_deviation=vc_lfo_deviation(&v->lfo3);
    vc_update_voice(s,v);vc_unison_trigger(v,&v->lfo,s->param[VC_LFO_TRIGGER]);
    vc_unison_trigger(v,&v->lfo2,s->param[VC_LFO2_TRIGGER]);vc_unison_trigger(v,&v->lfo3,0);
    unsigned phase=s->param[VC_INIT_PHASE];
    if(!phase){
        uint32_t random=v->noise.rng^0xa511e9b3u;
        v->osc[0].phase=vc_start_random(&random);
        v->osc[1].phase=vc_start_random(&random);
        v->osc3.phase=vc_start_random(&random);
        v->sub.phase=v->osc[0].phase*.5f; /* source He.reset halves phase */
    }else if(phase<128){v->osc[1].phase=phase/128.f;v->osc3.phase=1-phase/128.f;}
    v->env.stage=v->filter_env.stage=1;
}
/* Reuse the fixed four-voice DSP pool. Steal complete old unison groups. */
static void vc_poly_note_on(VCSynth *s,int n,int vel){
 if(n<0||n>127)return;
 if(vel<=0){vc_note_off(s,n);return;}
 unsigned count=s->param[VC_UNISON_MODE]+1,age=++s->counter;int previous=s->last_note;
 for(unsigned i=0;i<VC_VOICES;i++)if(s->voice[i].env.stage&&s->voice[i].note==n){s->voice[i].held=0;vc_voice_release(&s->voice[i]);}
 VCVoice *chosen[VC_VOICES];unsigned selected=0,used=0;
 while(selected<count){
  int slot=-1;for(unsigned i=0;i<VC_VOICES;i++)if(!(used&(1u<<i))&&!s->voice[i].env.stage){slot=i;break;}
  if(slot<0){for(unsigned i=0;i<VC_VOICES;i++)if(!(used&(1u<<i))&&(slot<0||s->voice[i].age<s->voice[slot].age))slot=i;
   unsigned old=s->voice[slot].age;
   for(unsigned i=0;i<VC_VOICES;i++)if(!(used&(1u<<i))&&(i==(unsigned)slot||(count>1&&s->voice[i].age==old))){
#ifdef VC_DUAL_RENDER
    vc_voice_fade(s,i);
#endif
    s->voice[i].env.stage=0;s->voice[i].held=0;}
  }
  used|=1u<<slot;chosen[selected++]=&s->voice[slot];
 }
 float phase=((int)s->param[VC_UNISON_PHASE]-64)/(s->param[VC_UNISON_PHASE]>=64?63.f:64.f)*.5f;
 for(unsigned i=0;i<count;i++){float side=count>1?2.f*i/(count-1)-1:0;
  vc_start_voice(s,chosen[i],n,vel,previous,side,count>1&&(count&1),count>1?i*phase/(count-1):0,age);
 }
 s->last_note=n;
}
/* Source key modes 1..4: last-note priority, one reusable voice. Ordered
 * deferred releases preserve sustain-pedal release order from the source Set. */
static void vc_note_remove(uint8_t *notes,unsigned *count,int note){
 for(unsigned i=0;i<*count;i++)if(notes[i]==note){
  /* Volatile byte loop: target runtime has no memmove implementation. */
  volatile uint8_t *dst=notes;for(unsigned j=i;j+1<*count;j++)dst[j]=dst[j+1];
  --*count;return;
 }
}
static void vc_mono_retarget(VCSynth *s,VCVoice *v,int note,float velocity,int glide,int retrigger){
#ifdef VC_MATRIX
 v->release_velocity=velocity;
#endif
 float start=glide?v->pitch:note;
#ifdef VC_MATRIX
 float previous_portamento=v->portamento_value,previous_coef=v->portamento_coef;
#endif
 v->generated_glide_left=0;
 v->note=note;v->pitch=start;v->glide_error=0;v->velocity=velocity;v->held=1;v->age=++s->counter;
 if(retrigger){
  v->lfo_deviation=vc_lfo_deviation(&v->lfo);
  v->lfo2_deviation=vc_lfo_deviation(&v->lfo2);
  v->lfo3_deviation=vc_lfo_deviation(&v->lfo3);
 }
 vc_update_voice(s,v);
#ifdef VC_MATRIX
 if(s->matrix){v->portamento_value=previous_portamento;v->portamento_coef=previous_coef;}
#endif
 if(retrigger){
#ifdef VC_DUAL_RENDER
  if(v->env.attack_inc>1.f/VC_VOICE_FADE)vc_voice_fade(s,(unsigned)(v-s->voice));
#endif
  v->env.stage=v->filter_env.stage=1;v->punch_at=0;v->lfo3_elapsed_frames=0;
  vc_unison_trigger(v,&v->lfo,s->param[VC_LFO_TRIGGER]);
  vc_unison_trigger(v,&v->lfo2,s->param[VC_LFO2_TRIGGER]);vc_unison_trigger(v,&v->lfo3,0);
 }
 v->control_remaining=0;s->last_note=note;
}
/* HOLD uses mono_notes as the physical keys-down set; deferred releases keep
 * pedal insertion order. A new phrase releases only the previously latched
 * notes, never notes still sustained by the pedal. No per-sample work. */
static void vc_hold_release(VCSynth *s,int n){
 for(int i=0;i<VC_VOICES;i++){VCVoice *v=&s->voice[i];
  if(v->env.stage&&v->held&&v->note==n){v->held=0;vc_voice_release(v);}
 }
}
static void vc_hold_note_on(VCSynth *s,int n,int vel){
 vc_note_remove(s->mono_deferred,&s->mono_deferred_count,n);
 if(!s->mono_count){
  for(int k=0;k<128;k++)if(s->hold_latched[k>>3]&(1u<<(k&7)))vc_hold_release(s,k);
  memset(s->hold_latched,0,sizeof(s->hold_latched));
 }
 vc_note_remove(s->mono_notes,&s->mono_count,n);
 s->mono_notes[s->mono_count++]=(uint8_t)n;
 vc_poly_note_on(s,n,vel);
}
static void vc_hold_note_off(VCSynth *s,int n){
 if(n<0||n>127)return;
 vc_note_remove(s->mono_notes,&s->mono_count,n);
 if(s->pedal){
  for(unsigned i=0;i<s->mono_deferred_count;i++)if(s->mono_deferred[i]==n)return;
  s->mono_deferred[s->mono_deferred_count++]=(uint8_t)n;
 }else s->hold_latched[n>>3]|=(uint8_t)(1u<<(n&7));
}
void vc_note_on(VCSynth *s,int n,int vel){
 if(n>=0&&n<=127&&vel>0)for(unsigned i=0;i<2;i++){
  unsigned phase=s->param[i?VC_LFO2_TRIGGER:VC_LFO_TRIGGER];
  if(s->param[i?VC_LFO2_GLOBAL:VC_LFO_GLOBAL]&&(phase||s->global_lfo[i].env_mode))
   vc_lfo_trigger(&s->global_lfo[i],phase);
 }

 if(!s->param[VC_KEY_MODE]){vc_poly_note_on(s,n,vel);return;}
 if(n<0||n>127)return;
 if(vel<=0){vc_note_off(s,n);return;}
 if(s->param[VC_KEY_MODE]==5){vc_hold_note_on(s,n,vel);return;}
 unsigned mode=s->param[VC_KEY_MODE];int legato=s->mono_count!=0;
 vc_note_remove(s->mono_deferred,&s->mono_deferred_count,n);
 vc_note_remove(s->mono_notes,&s->mono_count,n);
 s->mono_notes[s->mono_count++]=(uint8_t)n;
 int glide=mode==1||mode==3||legato;
 VCVoice *v=&s->voice[0];
 unsigned count=s->param[VC_UNISON_MODE]+1,ready=1;for(unsigned i=0;i<count;i++)if(!s->voice[i].env.stage)ready=0;
 if(ready){for(unsigned i=0;i<count;i++){v=&s->voice[i];vc_mono_retarget(s,v,n,vc_clip(vel/127.f,0,1),glide,mode<=2||!legato||!v->held);}}
 else{
  if(count>1){
#ifdef VC_DUAL_RENDER
   for(unsigned i=0;i<count;i++)if(s->voice[i].env.stage)vc_voice_fade(s,i);
#endif
   memset(s->voice,0,sizeof(s->voice));
  }
  if(!glide)s->last_note=-1;
  vc_poly_note_on(s,n,vel);
 }
}
static void vc_mono_note_off(VCSynth *s,int n){
 if(n<0||n>127)return;
 if(s->pedal){
  for(unsigned i=0;i<s->mono_deferred_count;i++)if(s->mono_deferred[i]==n)return;
  s->mono_deferred[s->mono_deferred_count++]=(uint8_t)n;return;
 }
 vc_note_remove(s->mono_notes,&s->mono_count,n);
 VCVoice *v=&s->voice[0];
 if(!v->env.stage||!v->held||v->note!=n)return;
 for(unsigned i=0;i<=s->param[VC_UNISON_MODE];i++){v=&s->voice[i];
  if(!s->mono_count){v->held=0;vc_voice_release(v);}
  else vc_mono_retarget(s,v,s->mono_notes[s->mono_count-1],v->velocity,1,s->param[VC_KEY_MODE]<=2);
 }
}
void vc_note_off(VCSynth *s,int n){
    if(s->param[VC_KEY_MODE]==5){vc_hold_note_off(s,n);return;}
    if(s->param[VC_KEY_MODE]){vc_mono_note_off(s,n);return;}
    for(int i=0;i<VC_VOICES;i++){VCVoice *v=&s->voice[i];if(v->note==n&&v->env.stage){v->held=0;if(!s->pedal)vc_voice_release(v);}}
}
/* A short per-voice glide leaves the patch's portamento and matrix alone. */
void vc_generated_slide_on(VCSynth *s,int n,int velocity,int previous){
 enum {SLIDE_TICKS=48};
 vc_note_on(s,n,velocity);
 for(unsigned i=0;i<VC_VOICES;i++){
  VCVoice *v=&s->voice[i];
  if(!v->env.stage||!v->held||v->note!=n)continue;
  v->pitch=(float)previous;
  v->control_remaining=0;
  v->generated_glide_delta=(n-previous)*(1.f/SLIDE_TICKS);
  v->generated_glide_left=SLIDE_TICKS;
 }
}

/* A generated gate releases on time even when the player's pedal is held.
 * HOLD mode still latches played keys; sequencer notes are released here. */
void vc_generated_note_off(VCSynth *s,int n,unsigned release_limit){
 unsigned pedal=s->pedal;
 s->pedal=0;
 vc_note_off(s,n);
 s->pedal=pedal;
 if(s->param[VC_KEY_MODE]==5){
  s->hold_latched[n>>3]&=(uint8_t)~(1u<<(n&7));
  vc_hold_release(s,n);
 }
 if(release_limit<127){
  float coefficient=expf(-1/fmaxf(1,vc_release[release_limit]*s->sample_rate));
  for(unsigned i=0;i<VC_VOICES;i++){
   VCVoice *v=&s->voice[i];
   if(v->note==n&&v->env.stage==4&&v->env.release_coef>coefficient)
    v->env.release_coef=coefficient;
  }
 }
}
#ifdef VC_MATRIX
void vc_note_off_velocity(VCSynth *s,int n,int velocity){
 /* A pedal/hold deferral releases later with velocity zero in the original. */
 unsigned released=0;
 for(unsigned i=0;i<VC_VOICES;i++)if(s->voice[i].note==n && s->voice[i].held && s->voice[i].env.stage)released|=1u<<i;
 vc_note_off(s,n);
 for(unsigned i=0;i<VC_VOICES;i++)if((released&(1u<<i)) && s->voice[i].note==n && !s->voice[i].held && s->voice[i].env.stage==4)
  s->voice[i].release_velocity=vc_clip(velocity,0,127)/127.f;
}
#endif
void vc_panic(VCSynth *s){
#ifdef VC_DUAL_RENDER
 memset(s->voice_last,0,sizeof(s->voice_last));memset(s->voice_fade_left,0,sizeof(s->voice_fade_left));
#endif
 memset(s->voice,0,sizeof(s->voice));s->pedal=0;s->mono_count=s->mono_deferred_count=0;memset(s->hold_latched,0,sizeof(s->hold_latched));}
void vc_bend(VCSynth *s,int x){
    s->bend=vc_clip((x-8192)/8192.f,-1,1);
    for(int i=0;i<VC_VOICES;i++)if(s->voice[i].env.stage){
#ifdef VC_MATRIX
     float value=s->voice[i].portamento_value,coefficient=s->voice[i].portamento_coef;
#endif
     vc_update_voice(s,&s->voice[i]);
#ifdef VC_MATRIX
     if(s->matrix){s->voice[i].portamento_value=value;s->voice[i].portamento_coef=coefficient;}
#endif
    }
}
/* Original CC numbers for implemented controls; retain native filter aliases. */
int vc_cc_param(int cc){
 switch(cc){
 case 5:return VC_PORTAMENTO;
 case 10:return VC_PAN;
 case 17:return VC_SHAPE1;
 case 18:return VC_PW1;
 case 20:return VC_TUNE1;
 case 21:return VC_KEYFOLLOW1;
 case 22:return VC_SHAPE2;
 case 23:return VC_PW2;
 case 25:return VC_TUNE2;
 case 26:return VC_DETUNE;
 case 27:return VC_FM_AMOUNT;
 case 28:return VC_SYNC;
 case 29:return VC_OSC2_ENV;
 case 30:return VC_FM_ENV;
 case 31:return VC_KEYFOLLOW2;
 case 33:return VC_BALANCE;
 case 34:return VC_SUB_VOLUME;
 case 35:return VC_SUB_SHAPE;
 case 36:return VC_SAT_DRIVE;
 case 37:return VC_NOISE_VOLUME;
 case 38:return VC_RING_VOLUME;
 case 39:return VC_NOISE_COLOR;
 case 40:return VC_CUTOFF;
 case 41:return VC_CUTOFF2;
 case 42:return VC_RESONANCE;
 case 43:return VC_RESONANCE2;
 case 44:return VC_F_AMOUNT1;
 case 45:return VC_F_AMOUNT2;
 case 46:return VC_F_KEYFOLLOW1;
 case 47:return VC_F_KEYFOLLOW2;
 case 48:return VC_FILTER_BALANCE;
 case 49:return VC_SAT_CURVE;
 case 51:return VC_FILTER_MODE;
 case 52:return VC_FILTER_MODE2;
 case 53:return VC_ROUTING;
 case 54:return VC_F_ATTACK;
 case 55:return VC_F_DECAY;
 case 56:return VC_F_SUSTAIN;
 case 57:return VC_F_SLOPE;
 case 58:return VC_F_RELEASE;
 case 59:return VC_ATTACK;
 case 60:return VC_DECAY;
 case 61:return VC_SUSTAIN;
 case 62:return VC_AMP_SLOPE;
 case 63:return VC_RELEASE;
 case 67:return VC_LFO_RATE;
 case 68:return VC_LFO_SHAPE;
 case 69:return VC_LFO_MODE;
 case 70:return VC_LFO_GLOBAL;
 case 72:return VC_LFO_KEYTRACK;
 case 73:return VC_LFO_TRIGGER;
 case 75:return VC_LFO_OSC2;
 case 76:return VC_LFO_PW;
 case 77:return VC_LFO_RESONANCE;
 case 78:return VC_LFO_FILTER_GAIN;
 case 79:return VC_LFO2_RATE;
 case 80:return VC_LFO2_SHAPE;
 case 81:return VC_LFO2_MODE;
 case 82:return VC_LFO2_GLOBAL;
 case 83:return VC_LFO2_CONTOUR;
 case 84:return VC_LFO2_KEYTRACK;
 case 85:return VC_LFO2_TRIGGER;
 case 86:return VC_LFO2_SHAPE_AMOUNT;
 case 87:return VC_LFO2_FM_AMOUNT;
 case 88:return VC_LFO2_CUTOFF1;
 case 89:return VC_LFO2_CUTOFF2;
 case 90:return VC_LFO2_PAN;
 case 91:return VC_PATCH_VOLUME;
 case 93:return VC_TRANSPOSE;
 case 94:return VC_KEY_MODE;
 case 71:return VC_RESONANCE;
 case 74:return VC_CUTOFF;
 default:return -1;}
}
void vc_cc(VCSynth *s,int cc,int v){
#ifdef VC_MATRIX
    int source=vc_matrix_cc_source((unsigned)cc);
    if(source>=0)s->controllers[source]=vc_clip(v,0,127)/127.f;
#endif
    /* Original boolean env CC clamps to 0/1; native mode also reserves 0 for OFF.
     * A mode CC explicitly enables CYCLE (1) or ENVELOPE (2). */
    if(cc==69||cc==81){vc_param(s,cc==69?VC_LFO_MODE:VC_LFO2_MODE,v>0?2:1);return;}
    int id=vc_cc_param(cc);if(id>=0){vc_param(s,(unsigned)id,(unsigned)vc_clip((float)v,0,127));return;}
    if(cc==7||cc==11){
        float value=vc_clip((float)v,0,127)/127.f;value*=value;
        if(cc==7)s->channel_volume=value;else s->expression=value;
        for(unsigned i=0;i<VC_VOICES;i++)if(s->voice[i].env.stage)vc_amp_update(s,&s->voice[i]);
    }
    else if(cc==120)vc_panic(s);
    else if(cc==123){if(s->param[VC_KEY_MODE]){s->mono_count=s->mono_deferred_count=0;memset(s->hold_latched,0,sizeof(s->hold_latched));for(int i=0;i<VC_VOICES;i++){s->voice[i].held=0;if(s->voice[i].env.stage)vc_voice_release(&s->voice[i]);}return;}for(int i=0;i<VC_VOICES;i++){s->voice[i].held=0;if(!s->pedal&&s->voice[i].env.stage)vc_voice_release(&s->voice[i]);}}
    else if(cc==64){s->pedal=v>=64;if(s->param[VC_KEY_MODE]){while(!s->pedal&&s->mono_deferred_count){int note=s->mono_deferred[0];vc_note_remove(s->mono_deferred,&s->mono_deferred_count,note);vc_note_off(s,note);}return;}if(!s->pedal)for(int i=0;i<VC_VOICES;i++)if(!s->voice[i].held&&s->voice[i].env.stage)vc_voice_release(&s->voice[i]);}
}
unsigned vc_active(const VCSynth *s){unsigned n=0;for(int i=0;i<VC_VOICES;i++)n+=s->voice[i].env.stage!=0;return n;}
static inline __attribute__((always_inline)) void vc_render_voice_sample(VCSynth *s,VCVoice *v,unsigned i,const float global[3],float *out_l,float *out_r){
#ifndef VC_BAND_CACHE
 (void)i;
#endif
 if(v->control_remaining<=0){
  vc_filter_control_at(s,v,global);
#ifdef VC_BAND_CACHE
  void (*volatile bind)(VCSynth*,unsigned)=vc_band_voice_control;bind(s,i);
#endif
  v->control_remaining=VC_CONTROL_FRAMES;
 }
 float sync=s->param[VC_SYNC]&&v->osc[0].phase+v->inc[0]>=1?(1-v->osc[0].phase)/v->inc[0]:-1;
 float phase=v->osc[0].phase,raw1=vc_osc_next(&v->osc[0],v->inc[0],-1),inc2=v->inc[1];
 if(v->fm_depth>0){
  if(s->param[VC_FM_MODE]==3)v->fm_noise_sample=vc_noise_next(&v->noise);
  inc2=vc_fm_increment(v,s->param[VC_FM_MODE],phase,raw1);
 }
 float raw2=vc_osc_next(&v->osc[1],inc2,sync);
 float o1,o2;vc_mix_sources(s,v,raw1,raw2,v->source_gain[3],v->source_gain[4],&o1,&o2);
 if(v->osc3_gain>0)o2+=vc_osc_next(&v->osc3,v->inc3,-1)*v->osc3_gain;
 float vl,vr;vc_route_sample(s,v,o1,o2,&vl,&vr);
 float amp=vc_env_next(&v->env)*vc_amp_gain_next(v)*(.16f/.631f);
 vc_env_next(&v->filter_env);v->control_remaining--;
 *out_l=vl*amp;*out_r=vr*amp;
}static inline __attribute__((always_inline)) void vc_render_voice_sample_fused(VCSynth *s,VCVoice *v,unsigned i,const float global[3],float *out_l,float *out_r){
#ifndef VC_BAND_CACHE
 (void)i;
#endif
 if(v->control_remaining<=0){
  vc_filter_control_at(s,v,global);
#ifdef VC_BAND_CACHE
  void (*volatile bind)(VCSynth*,unsigned)=vc_band_voice_control;bind(s,i);
#endif
  v->control_remaining=VC_CONTROL_FRAMES;
 }
 float sync=s->param[VC_SYNC]&&v->osc[0].phase+v->inc[0]>=1?(1-v->osc[0].phase)/v->inc[0]:-1;
 float phase=v->osc[0].phase,raw1=vc_osc_next_inline(&v->osc[0],v->inc[0],-1),inc2=v->inc[1];
 if(v->fm_depth>0){
  if(s->param[VC_FM_MODE]==3)v->fm_noise_sample=vc_noise_next_inline(&v->noise);
  inc2=vc_fm_increment_inline(v,s->param[VC_FM_MODE],phase,raw1);
 }
 float raw2=vc_osc_next_inline(&v->osc[1],inc2,sync);
 float o1,o2;vc_mix_sources_inline(s,v,raw1,raw2,v->source_gain[3],v->source_gain[4],&o1,&o2);
 if(v->osc3_gain>0)o2+=vc_osc_next_inline(&v->osc3,v->inc3,-1)*v->osc3_gain;
 float vl,vr;vc_route_sample_inline(s,v,o1,o2,&vl,&vr);
 float amp=vc_env_next_inline(&v->env)*vc_amp_gain_next(v)*(.16f/.631f);
 vc_env_next_inline(&v->filter_env);v->control_remaining--;
 *out_l=vl*amp;*out_r=vr*amp;
}/* One voice over one control span. Same per-sample code as
 * vc_render_voice_sample with its helpers inlined; out is the voice's own
 * buffer and cannot alias voice or synth state. */
static VC_HOT __attribute__((noinline)) void vc_render_voice_span(VCSynth *s,unsigned i,const float *global,float (*__restrict out)[2],unsigned from,unsigned to){
 VCVoice *v=&s->voice[i];
 for(unsigned j=from;j<to;j++){
  float l=0,r=0;
  if(v->env.stage)vc_render_voice_sample_fused(s,v,i,global,&l,&r);
  out[j][0]=l;out[j][1]=r;
 }
}

static inline __attribute__((always_inline)) void vc_render_mix(VCSynth *s,float *left,float *right,unsigned j,float l,float r){
#ifndef VC_POST_FX
 (void)s;
#else
 if(s->distortion.mode){l=vc_distortion_next(&s->distortion,l,0);r=vc_distortion_next(&s->distortion,r,1);}
 l=vc_boost_next(&s->boost,l,0);r=vc_boost_next(&s->boost,r,1);
 if(s->param[VC_EQ_ENABLE]){l=vc_eq_next(&s->eq,l,0);r=vc_eq_next(&s->eq,r,1);}
#endif
 if(right){left[j]=l;right[j]=r;}else left[j]=(l+r)*.5f;
}
#ifdef VC_DUAL_RENDER
/* Per-voice render buffers: 256 frames keeps 12 voices inside the pool.
 * vc_render_stereo renders a 512-frame block as two 256-frame halves. */
#define VC_DUAL_FRAMES 256u
#ifdef PLAT_HOST
#define VC_RENDER_POOL
#else
#define VC_RENDER_POOL __attribute__((section(".pool")))
#endif
static float vc_voice_audio[VC_VOICES][VC_DUAL_FRAMES][2] VC_RENDER_POOL;
typedef struct {
 unsigned end;
 float global[3];
} VCControlSpan;
typedef struct {
 VCSynth *synth;
 unsigned first,last,span_count,active;
 VCControlSpan spans[VC_DUAL_FRAMES/VC_CONTROL_FRAMES+1];
} VCRenderJob;
static VCRenderJob vc_render_job;

static VC_HOT __attribute__((noinline)) void vc_render_voice_range(VCRenderJob *job,unsigned first,unsigned last){
 VCSynth *s=job->synth;
 for(unsigned i=first;i<last;i++){
  if(!(job->active&(1u<<i)))continue;
  VCVoice *v=&s->voice[i];
  unsigned j=0;
  for(unsigned k=0;k<job->span_count;k++){
   const VCControlSpan *span=&job->spans[k];
   vc_render_voice_span(s,i,span->global,vc_voice_audio[i],j,span->end);
   j=span->end;
  }
 }
}
static VC_HOT void vc_render_worker(void *arg){
 VCRenderJob *job=arg;
 vc_render_voice_range(job,job->first,job->last);
}

#ifdef PLAT_HOST
/* Only the mono test adapter (right==NULL) uses this merge; the FM-1 app
 * always renders stereo, so it is host-only. */
static VC_HOT __attribute__((noinline)) void vc_render_merge(VCSynth *s,float *left,float *right,unsigned frames,unsigned active){
 for(unsigned j=0;j<frames;j++){
  float l=0,r=0;
  for(unsigned i=0;i<VC_VOICES;i++)if(active&(1u<<i)){
   l+=vc_voice_audio[i][j][0];r+=vc_voice_audio[i][j][1];
  }
  vc_render_mix(s,left,right,j,l,r);
 }
}

/* Applied after the worker joins. Steals fade out the previous voice sample
 * while the replacement starts, even if the old unison group loses a slot. */
static unsigned vc_render_voice_fades(VCSynth *s,unsigned active,unsigned frames){
 for(unsigned i=0;i<VC_VOICES;i++){
  int sounding=(active&(1u<<i))!=0;
  unsigned left=s->voice_fade_left[i];
  if(left){
   if(!sounding)memset(vc_voice_audio[i],0,frames*sizeof(vc_voice_audio[i][0]));
   for(unsigned j=0;j<frames&&left;j++,left--){
    float old=(float)left*(1.f/VC_VOICE_FADE),current=1-old;
    for(unsigned ch=0;ch<2;ch++)
     vc_voice_audio[i][j][ch]=old*s->voice_fade_from[i][ch]+current*vc_voice_audio[i][j][ch];
   }
   s->voice_fade_left[i]=left;
   active|=1u<<i;sounding=1;
  }
  for(unsigned ch=0;ch<2;ch++)s->voice_last[i][ch]=sounding?vc_voice_audio[i][frames-1][ch]:0;
 }
 return active;
}
#endif

/* Exact per-channel post stage. The voice sum, steal fades, distortion, boost
 * and EQ keep separate state for each channel, so CPU0 can run the left
 * channel while CPU1 runs the right one, with every operation and its order
 * unchanged. During that stage each core reads the voice buffers and the
 * shared settings but writes only its own 64-byte-aligned scratch: its copy of
 * the effect state, its output buffer and its voice_last values. CPU0 copies
 * CPU1's channel back after the join. */
typedef uint32_t __attribute__((may_alias)) vc_word;
#define VC_COPY_WORDS(to,from) do{vc_word *d_=(vc_word*)(void*)&(to);const vc_word *s_=(const vc_word*)(const void*)&(from); \
 for(unsigned w_=0;w_<sizeof(to)/4;w_++){__asm__ volatile("":::);d_[w_]=s_[w_];}}while(0)
typedef struct {
 VCDistortion distortion;VCBoost boost;VCEQ eq;
 float last[VC_VOICES];
 float out[VC_DUAL_FRAMES];
} __attribute__((aligned(64))) VCPostChannel;
static VCPostChannel vc_post[2] VC_RENDER_POOL;
typedef struct {
 VCSynth *synth;unsigned frames,active,fading,channel;
} __attribute__((aligned(64))) VCPostJob;
static VCPostJob vc_post_job;

static VC_HOT __attribute__((noinline)) void vc_post_channel(VCSynth *s,unsigned ch,unsigned frames,unsigned active,unsigned fading){
 VCPostChannel *o=&vc_post[ch];
 VC_COPY_WORDS(o->distortion,s->distortion);VC_COPY_WORDS(o->boost,s->boost);VC_COPY_WORDS(o->eq,s->eq);
 const unsigned dist=s->distortion.mode!=0,eq=s->param[VC_EQ_ENABLE]!=0;
 const unsigned mix=active|fading;
 /* Voice sum, voice by voice into out[]: each frame still adds the voices in
  * index order starting from 0, exactly as vc_render_merge did. */
 float *__restrict out=o->out;
 for(unsigned j=0;j<frames;j++)out[j]=0;
 for(unsigned i=0;i<VC_VOICES;i++)if(mix&(1u<<i)){
  const float (*__restrict buf)[2]=vc_voice_audio[i];
  const unsigned sounding=(active&(1u<<i))!=0;
  unsigned left=s->voice_fade_left[i],j=0;
  /* TEST: vc_render_voice_fades. A fading voice that stopped sounding was
   * zeroed before its fade. */
  if(left){const float from=s->voice_fade_from[i][ch];
   for(;j<frames&&j<left;j++){float b=sounding?buf[j][ch]:0.f;float old=(float)(left-j)*(1.f/VC_VOICE_FADE),current=1-old;out[j]+=old*from+current*b;if(j==frames-1)o->last[i]=old*from+current*b;}
  }
  if(j<frames){
   if(sounding){for(;j<frames;j++)out[j]+=buf[j][ch];o->last[i]=buf[frames-1][ch];}
   else {for(;j<frames;j++)out[j]+=0.f;o->last[i]=0;}
  }
 }
 for(unsigned j=0;j<frames;j++){
  float x=out[j];
  if(dist)x=vc_distortion_next(&o->distortion,x,ch);
  x=vc_boost_next(&o->boost,x,ch);
  if(eq)x=vc_eq_next(&o->eq,x,ch);
  out[j]=x;
 }
}
static VC_HOT void vc_post_worker(void *arg){
 VCPostJob *job=arg;
 vc_post_channel(job->synth,job->channel,job->frames,job->active,job->fading);
}
/* Fold one channel's post-stage results back into the synth. */
static void vc_post_commit(VCSynth *s,unsigned ch,float *out,unsigned frames,unsigned mix){
 const VCPostChannel *o=&vc_post[ch];
 s->distortion.previous[ch]=o->distortion.previous[ch];s->distortion.filtered[ch]=o->distortion.filtered[ch];
 s->distortion.held[ch]=o->distortion.held[ch];s->distortion.counter[ch]=o->distortion.counter[ch];
 s->boost.history[ch][0]=o->boost.history[ch][0];s->boost.history[ch][1]=o->boost.history[ch][1];
 vc_eq_commit_channel(&s->eq,&o->eq,ch);
 for(unsigned i=0;i<VC_VOICES;i++)s->voice_last[i][ch]=(mix&(1u<<i))?o->last[i]:0;
 memcpy(out,o->out,frames*sizeof(*out));
}

static __attribute__((noinline)) void vc_render_dual_block(VCSynth *s,float *left,float *right,unsigned frames){
 unsigned active=0,count=0;
 for(unsigned i=0;i<VC_VOICES;i++)if(s->voice[i].env.stage){active|=1u<<i;count++;}
 /* Put the extra voice of an odd group on Core 1. Core 0 also mixes
  * the output, runs stereo FX and services the audio interrupt. */
 unsigned mid=VC_VOICES;
 if(count>=2){
  unsigned prefix=0;
  mid=0;
  do{prefix+=(active>>mid)&1u;mid++;}while(prefix<count/2);
 }

 VCRenderJob *job=&vc_render_job;
 job->synth=s;job->first=mid;job->last=VC_VOICES;job->span_count=0;job->active=active;
 for(unsigned j=0;j<frames;){
  if(!s->global_remaining)s->global_remaining=VC_CONTROL_FRAMES;
  unsigned n=frames-j;if(n>s->global_remaining)n=s->global_remaining;
  VCControlSpan *span=&job->spans[job->span_count++];
  float (*volatile tick)(VCLfo*,unsigned)=vc_lfo_tick;
  for(unsigned i=0;i<3;i++)span->global[i]=tick(&s->global_lfo[i],n);
  s->global_remaining-=n;
  j+=n;span->end=j;
 }
 /* CPU0 joins before applying stereo FX, publishing table requests or
  * returning to the main loop. Both renderers only write their own voices. */
 int submitted=mid<VC_VOICES&&dc_submit(vc_render_worker,job);
 void (*volatile range)(VCRenderJob*,unsigned,unsigned)=vc_render_voice_range;
 range(job,0,submitted?mid:VC_VOICES);
 if(submitted&&!dc_wait()){
  vc_panic(s);
  for(unsigned j=0;j<frames;j++){left[j]=0;if(right)right[j]=0;}
  return;
 }
 if(!right){
#ifdef PLAT_HOST
  void (*volatile merge)(VCSynth*,float*,float*,unsigned,unsigned)=vc_render_merge;
  merge(s,left,right,frames,vc_render_voice_fades(s,active,frames));
#else
  for(unsigned j=0;j<frames;j++)left[j]=0;   /* mono is a host test adapter only */
#endif
  return;
 }
 unsigned fading=0;
 for(unsigned i=0;i<VC_VOICES;i++)if(s->voice_fade_left[i])fading|=1u<<i;
 VCPostJob *post=&vc_post_job;
 post->synth=s;post->frames=frames;post->active=active;post->fading=fading;post->channel=1;
 int split=dc_submit(vc_post_worker,post);
 void (*volatile channel)(VCSynth*,unsigned,unsigned,unsigned,unsigned)=vc_post_channel;
 channel(s,0,frames,active,fading);
 if(!split)channel(s,1,frames,active,fading);
 else if(!dc_wait()){
  vc_panic(s);
  for(unsigned j=0;j<frames;j++){left[j]=0;right[j]=0;}
  return;
 }
 const unsigned mix=active|fading;
 vc_post_commit(s,0,left,frames,mix);
 vc_post_commit(s,1,right,frames,mix);
 for(unsigned i=0;i<VC_VOICES;i++){unsigned l=s->voice_fade_left[i];s->voice_fade_left[i]=l>frames?l-frames:0;}
}

#endif
#ifdef VC_FLOAT_HASH
/* Test-only: hash of every rendered float sample, for bit-exact comparisons. */
#include <stdio.h>
#include <stdlib.h>
static unsigned long long vc_hash_value=1469598103934665603ull;
static unsigned long long vc_hash_frames;
static FILE *vc_hash_dump;
static void vc_hash_report(void){fprintf(stderr,"VC_FLOAT_HASH %016llx frames %llu\n",vc_hash_value,vc_hash_frames);}
static void vc_hash_block(const float *l,const float *r,unsigned n){
 static int once;if(!once){once=1;atexit(vc_hash_report);const char *d=getenv("VC_HASH_DUMP");if(d)vc_hash_dump=fopen(d,"wb");}
 for(unsigned i=0;i<n;i++){uint32_t a,b=0;memcpy(&a,&l[i],4);if(r)memcpy(&b,&r[i],4);
  vc_hash_value=(vc_hash_value^a)*1099511628211ull;vc_hash_value=(vc_hash_value^b)*1099511628211ull;
  if(vc_hash_dump){fwrite(&a,4,1,vc_hash_dump);fwrite(&b,4,1,vc_hash_dump);}}
 vc_hash_frames+=n;
}
#endif
/* right==NULL is the mono test adapter; the app always renders both channels. */
void vc_render_stereo(VCSynth *s,float *left,float *right,unsigned frames){
#ifdef VC_FLOAT_HASH
    float *hash_l=left,*hash_r=right;unsigned hash_n=frames;
#endif
#ifdef VC_BAND_CACHE
    void (*volatile boundary)(VCSynth*)=vc_band_voice_boundary;boundary(s);
#endif
#ifdef VC_DUAL_RENDER
    void (*volatile publish)(VCSynth*)=vc_band_voice_publish;
    void (*volatile render_block)(VCSynth*,float*,float*,unsigned)=vc_render_dual_block;
    while(frames>VC_DUAL_FRAMES){
      unsigned count=VC_DUAL_FRAMES;
      if(s->global_remaining)count-=VC_CONTROL_FRAMES-s->global_remaining;
      render_block(s,left,right,count);
      left+=count;if(right)right+=count;frames-=count;
    }
    if(frames)render_block(s,left,right,frames);
    publish(s);
#ifdef VC_FLOAT_HASH
    vc_hash_block(hash_l,hash_r,hash_n);
#endif
#else
    unsigned j=0;
    while(j<frames){
      if(!s->global_remaining)s->global_remaining=VC_CONTROL_FRAMES;
      unsigned count=frames-j;if(count>s->global_remaining)count=s->global_remaining;
      vc_lfo_tick(&s->global_lfo[0],count);vc_lfo_tick(&s->global_lfo[1],count);vc_lfo_tick(&s->global_lfo[2],count);
      s->global_remaining-=count;unsigned end=j+count;
      const float global[]={s->global_lfo[0].value,s->global_lfo[1].value,s->global_lfo[2].value};
      for(;j<end;j++){
        float l=0,r=0;
        for(int i=0;i<VC_VOICES;i++){
            VCVoice *v=&s->voice[i];if(!v->env.stage)continue;
            float vl,vr;
            vc_render_voice_sample(s,v,(unsigned)i,global,&vl,&vr);
            l+=vl;r+=vr;
        }
        vc_render_mix(s,left,right,j,l,r);
      }
    }
#endif
}
void vc_render(VCSynth *s,float *mono,unsigned frames){vc_render_stereo(s,mono,0,frames);}

/* Isolated, muted hardware workloads. Their loop/setup costs differ; compare
 * with FULL and IDLE rather than treating component percentages as additive. */
void vc_profile_prepare(VCSynth *s){
 vc_init(s,44100);vc_param(s,VC_SUB_VOLUME,17);vc_param(s,VC_RING_VOLUME,13);
 static const int notes[4]={48,55,60,64};
 for(int i=0;i<VC_VOICES;i++){
  vc_note_on(s,notes[i%4]+12*(i/4),100);VCVoice *v=&s->voice[i];
  v->env.stage=v->filter_env.stage=3;v->env.level=v->env.sustain;
  v->filter_env.level=v->filter_env.sustain;
 }
}
static volatile float vc_profile_sink;
void vc_profile_run(VCSynth *s,unsigned mode,unsigned frames){
 float acc=0;
 if(mode==0){
  float l[32],r[32];
  void (*volatile render)(VCSynth*,float*,float*,unsigned)=vc_render_stereo;
  while(frames){unsigned n=frames>32?32:frames;render(s,l,r,n);acc+=l[n-1]+r[n-1];frames-=n;}
 }else if(mode==1){
  for(unsigned j=0;j<frames;j++)for(int i=0;i<VC_VOICES;i++){
   VCVoice *v=&s->voice[i];
   float sync=s->param[VC_SYNC]&&v->osc[0].phase+v->inc[0]>=1?(1-v->osc[0].phase)/v->inc[0]:-1;
   float a=vc_osc_next(&v->osc[0],v->inc[0],-1),b=vc_osc_next(&v->osc[1],v->inc[1],sync),x,y;
   vc_mix_sources(s,v,a,b,1-.003219f*64,.794f,&x,&y);acc+=x+y;
  }
 }else if(mode==2){
  for(unsigned j=0;j<frames;j++)for(int i=0;i<VC_VOICES;i++){
   VCVoice *v=&s->voice[i];if(!v->filter[0].configured)vc_filter_control(s,v);
   float l,r;vc_route_sample(s,v,(j&16)?.125f:-.125f,(j&8)?.1f:-.1f,&l,&r);acc+=l+r;
  }
 }else if(mode==3){
  for(unsigned j=0;j<frames;j++)for(int i=0;i<VC_VOICES;i++){
   VCVoice *v=&s->voice[i];float amp=vc_env_next(&v->env)*vc_amp_gain_next(v)*(.16f/.631f);
   acc+=amp+vc_env_next(&v->filter_env);
  }
 }else if(mode==4){
  for(unsigned j=0;j<frames;j++)for(int i=0;i<VC_VOICES;i++){
   VCVoice *v=&s->voice[i];if(v->control_remaining<=0){vc_filter_control(s,v);v->control_remaining=VC_CONTROL_FRAMES;}
   v->control_remaining--;
  }
  acc=s->voice[0].filter[0].g;
 }
 vc_profile_sink=acc;
}

/* Atomic muted preset preparation: apply each clamped value once, then derive
 * coefficients once. Caller must keep audio muted until waveform caches match. */
void vc_apply_patch(VCSynth *s,const uint8_t *p){
 float sr=s->sample_rate;vc_init(s,sr);
 for(unsigned id=0;id<VC_PARAMS;id++){unsigned max=vc_param_max(id);s->param[id]=p[id]>max?max:p[id];}
 vc_route_setup(s);vc_global_config(s);
#ifdef VC_POST_FX
 vc_distortion_set(&s->distortion,s->param[VC_DIST_MODE],s->param[VC_DIST_LEVEL]);
 vc_boost_set(&s->boost,s->param[VC_BOOST_LEVEL],s->param[VC_BOOST_TUNE]);
 vc_eq_set(&s->eq,s->param[VC_EQ_LOW_GAIN],s->param[VC_EQ_LOW_FREQ],s->param[VC_EQ_MID_GAIN],s->param[VC_EQ_MID_FREQ],s->param[VC_EQ_MID_Q],s->param[VC_EQ_HIGH_GAIN],s->param[VC_EQ_HIGH_FREQ]);
#endif
}
