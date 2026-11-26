#include "band_voice.h"
#include <string.h>
#include "../base/include/fastmath.h"
/* Capture the old value while its slot is still pinned. The finite correction
 * removes the replacement step and reaches the exact new table after 128 samples. */
static float sample(const float *t,float phase){float at=(phase-fm_floorf(phase))*2048;unsigned i=(unsigned)at;return t[i]+(t[i+1]-t[i])*(at-i);}
static void select_table(VCOsc *o,const float *table){
 if(o->table==table)return;
#ifdef VC_BAND_CACHE
 float old=sample(o->table?o->table:vc_band_silence,o->phase);
 old+=o->table_correction*(o->table_ramp*(1.f/128.f));
 o->table_correction=old-sample(table,o->phase);o->table_ramp=128;
#endif
 o->table=table;
}
const float vc_band_silence[2049]={0};
static VCBandVoiceBinding *binding;
void vc_band_voice_attach(VCBandVoiceBinding *b,VCBandHandoff *h,VCSynth *s){memset(b,0,sizeof(*b));b->handoff=h;b->synth=s;binding=b;
#ifdef VC_DUAL_RENDER
 for(unsigned i=0;i<VC_BAND_CLIENTS;i++)b->desired[i]=VC_BAND_OFF;
#endif
}
void vc_band_voice_detach(void){if(binding)for(unsigned i=0;i<VC_BAND_CLIENTS;i++)vc_band_audio_release(binding->handoff,i);binding=0;}
/* Each voice renderer owns three clients. The cross-core path stages
 * requests per client; CPU0 publishes them after both renderers join. */
static void vc_band_voice_release(VCBandVoiceBinding *b,unsigned client){
#ifdef VC_DUAL_RENDER
 b->desired[client]=VC_BAND_OFF;
#else
 vc_band_audio_release(b->handoff,client);
#endif
}
static void vc_band_voice_request(VCBandVoiceBinding *b,unsigned client,unsigned wave,unsigned band){
#ifdef VC_DUAL_RENDER
 b->cold[client]=!b->ready[client];
 b->desired[client]=wave*14+band;
#else
 vc_band_word_publish_changed(&b->handoff->cold[client],!b->ready[client]);
 vc_band_audio_request(b->handoff,client,wave,band);
#endif
}
#ifdef VC_DUAL_RENDER
void vc_band_voice_publish(VCSynth *s){
 if(!binding||binding->synth!=s)return;
 for(unsigned i=0;i<VC_BAND_CLIENTS;i++){
  unsigned key=binding->desired[i];
  if(key==VC_BAND_OFF)vc_band_audio_release(binding->handoff,i);
  else{
   vc_band_word_publish_changed(&binding->handoff->cold[i],binding->cold[i]);
   vc_band_audio_request(binding->handoff,i,key/14,key%14);
  }
 }
}
#endif
void vc_band_voice_control(VCSynth *s,unsigned voice){
 if(!binding||binding->synth!=s||voice>=VC_VOICES)return;
 VCBandHandoff *h=binding->handoff;VCVoice *v=&s->voice[voice];
 if(!s->matrix){v->osc[0].wave=s->param[VC_WAVE1];v->osc[1].wave=s->param[VC_WAVE2];}
 if(s->param[VC_OSC3_MODE]==1)v->osc3.wave=v->osc[1].wave;
 for(unsigned osc=0;osc<3;osc++){
  unsigned client=voice*3+osc;VCOsc *o=osc<2?&v->osc[osc]:&v->osc3;
  if(!v->env.stage||o->wave_gain<=0||(osc==2&&(!s->param[VC_OSC3_MODE]||v->osc3_gain<=0))){vc_band_voice_release(binding,client);binding->ready[client]=0;select_table(o,vc_band_silence);continue;}
  if(binding->age[client]!=v->age){binding->age[client]=v->age;binding->ready[client]=0;}
  if(o->wave<0||o->wave>=64||o->band<0||o->band>=14){vc_band_voice_release(binding,client);select_table(o,vc_band_silence);binding->ready[client]=0;continue;}
  unsigned key=(unsigned)o->wave*14+(unsigned)o->band;vc_band_voice_request(binding,client,(unsigned)o->wave,(unsigned)o->band);
  if(h->audio_key[client]==key){select_table(o,h->audio_table[client]);binding->ready[client]=1;}
  else {
   unsigned published=h->audio_key[client];
   int safe=published!=VC_BAND_OFF&&published%14>=(unsigned)o->band;
   /* A stolen voice must not start with the previous note's different wave. */
   safe=safe&&(binding->ready[client]||published/14==(unsigned)o->wave);
   select_table(o,safe?h->audio_table[client]:vc_band_silence);
   if(safe)binding->ready[client]=1;
  }
 }
}
void vc_band_voice_boundary(VCSynth *s){
 if(!binding||binding->synth!=s)return;vc_band_audio_begin(binding->handoff);
 for(unsigned i=0;i<VC_VOICES;i++)vc_band_voice_control(s,i);
 vc_band_audio_end(binding->handoff);
}

/* Original control-rate wave selection, with original route-amount ordering
 * and discrete rounding. No reconstruction or cache mutation here. */
void vc_band_wave_control(const VCMatrix *m,const float *sources,const uint8_t *p,VCOsc *osc){
 for(unsigned i=0;i<2;i++){
  unsigned destination=i?13:8,base=p[i?VC_WAVE2:VC_WAVE1],wave=base;
  for(unsigned k=0;k<11;k++)if(m->route[k].destination==destination&&m->route[k].source>0&&m->route[k].source<28){
   wave=vc_matrix_rounded(m,sources,destination,base);break;
  }
  osc[i].wave=wave>63?63:(int)wave;
 }
}
