/* Experimental device backend: band-zero scratch is for muted LFO loading only. */
#include "../engine/wave_cache.h"
#include "wavetable_bank.h"
#include "wavetable_voice.h"
#include <string.h>
#include "../engine/wave_bank.inc"
#ifdef PLAT_HOST
#define POOL
#else
#define POOL __attribute__((section(".pool")))
#endif
static float tables[VC_BAND_SLOTS][2049] POOL;
static float lfo_table[2049] POOL;
static double coefficients[256];
static VCBandRebuild builder;
static VCBandBank bank;
static VCBandHandoff handoff;
static VCBandVoiceBinding binding;
static int loaded=-1,building=-1;
static unsigned prepare_active,prepare_index,prepare_selected[3];
const uint8_t *vc_wave_seed(void){return vc_bank_seed;}
unsigned vc_wave_seed_size(void){return sizeof(vc_bank_seed);}
int vc_wave_bank_valid(const uint8_t *p,unsigned n){VCBandBank b;return vc_band_bank_init(&b,p,n);}
int vc_wave_init(const uint8_t *p,unsigned n){
 loaded=building=-1;prepare_active=0;memset(&builder,0,sizeof(builder));
 if(!vc_band_bank_init(&bank,p,n))return 0;
 vc_band_handoff_init(&handoff,tables,vc_bank_sines,vc_band_bank_load,&bank);return 1;
}
void vc_band_device_attach(VCSynth *s){vc_band_voice_attach(&binding,&handoff,s);}
void vc_band_device_service(unsigned budget){vc_band_background(&handoff,budget);}
int vc_wave_begin(unsigned slot,unsigned wave){
 float peak;if(slot>2||!vc_band_bank_load(&bank,wave,coefficients,&peak))return 0;
 loaded=-1;building=(int)wave;return vc_band_begin(&builder,coefficients,vc_bank_sines,peak,0,lfo_table);
}
int vc_wave_service(unsigned budget){int r=vc_band_service(&builder,budget);if(r==1)loaded=building;return r;}
int vc_wave_cached(unsigned slot){return slot==2?loaded:-1;}
const float *const *vc_wave_tables(unsigned wave){(void)wave;return 0;}
const float *vc_wave_table(unsigned wave,unsigned band){return loaded==(int)wave&&band==0?lfo_table:vc_band_silence;}
unsigned vc_wave_progress(void){return builder.active?builder.sample*100/2048:100;}
uint32_t vc_wave_bytes_crc(const void *data,unsigned n){const uint8_t *p=data;uint32_t c=~0u;while(n--){c^=*p++;for(unsigned i=0;i<8;i++)c=(c>>1)^(0xedb88320u&(0u-(c&1)));}return ~c;}

/* Main-loop only, after audio acknowledges mute. Reset is safe because no
 * render hooks execute until wave_state returns to zero. Eighteen keys fit
 * within thirty-two slots; duplicate waves reuse already completed bands. */
int vc_band_device_prepare(const unsigned waves[3],unsigned budget){
 if(!prepare_active||memcmp(prepare_selected,waves,sizeof(prepare_selected))){
  VCBandSlot saved[VC_BAND_SLOTS];memcpy(saved,handoff.cache.slots,sizeof(saved));
  memcpy(prepare_selected,waves,sizeof(prepare_selected));prepare_index=0;prepare_active=1;
  vc_band_handoff_init(&handoff,tables,vc_bank_sines,vc_band_bank_load,&bank);
  /* Only completed bytes survive muted reset. Protect every requested band
   * up front, not just the one currently being prepared, from eviction. */
  for(unsigned i=0;i<VC_BAND_SLOTS;i++)if(saved[i].state==2){
   saved[i].refs=0;
   for(unsigned w=0;w<3;w++)if(saved[i].wave==(int)waves[w]&&(saved[i].band<5||saved[i].band==13))saved[i].refs=1;
   handoff.cache.slots[i]=saved[i];
  }
  vc_band_voice_attach(&binding,&handoff,binding.synth);
 }
 while(prepare_index<18){
 unsigned wave=prepare_selected[prepare_index/6],band=prepare_index%6==0?13:prepare_index%6-1;
 if(!vc_band_cache_request(&handoff.cache,0,wave,band)){prepare_active=0;return -1;}
 if(!vc_band_cache_commit(&handoff.cache,0)){
  if(!budget)return 0;
  if(vc_band_cache_service(&handoff.cache,budget)<0){prepare_active=0;return -1;}
  budget=0; /* At most one reconstruction service per preparation call. */
  if(!vc_band_cache_commit(&handoff.cache,0))return 0;
 }
 int slot=handoff.cache.clients[0].slot;
 vc_band_cache_release(&handoff.cache,0);
 handoff.cache.slots[slot].refs=1;
 prepare_index++;
 }
 for(unsigned i=0;i<VC_BAND_SLOTS;i++)handoff.cache.slots[i].refs=0;
 for(unsigned i=0;i<3;i++)handoff.cache.reserved[i]=(int)waves[i];
 prepare_active=0;return 1;
}

/* Main-loop snapshot: no reads of audio-owned counters or pointers. */
void vc_band_device_stats(unsigned out[6]){
 VCBandCache *c=&handoff.cache;out[0]=out[1]=0;
 for(unsigned i=0;i<VC_BAND_SLOTS;i++)out[0]+=c->slots[i].state==2;
 for(unsigned i=0;i<VC_BAND_CLIENTS;i++){
  VCBandClient *v=&c->clients[i];if(v->wave<0)continue;
  int slot=v->slot;
  if(slot<0||c->slots[slot].wave!=v->wave||c->slots[slot].band!=v->band)out[1]++;
 }
 out[2]=c->completed;out[3]=c->cancelled;out[4]=c->failures;
 out[5]=c->building<0?100:c->builder.sample*100/2048;
}
