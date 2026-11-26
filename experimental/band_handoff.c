#include "band_handoff.h"
#include <string.h>
void vc_band_handoff_init(VCBandHandoff *h,float tables[VC_BAND_SLOTS][2049],const double *sines,VCBandLoad load,void *ctx){
 memset(h,0,sizeof(*h));vc_band_cache_init(&h->cache,tables,sines,load,ctx);
 vc_band_word_init(&h->published,0);vc_band_word_init(&h->acknowledged,0);
 for(unsigned i=0;i<VC_BAND_CLIENTS;i++){vc_band_word_init(&h->cold[i],0);vc_band_word_init(&h->desired[i],VC_BAND_OFF);h->audio_key[i]=VC_BAND_OFF;h->proposed_slot[i]=-1;}
}
int vc_band_audio_request(VCBandHandoff *h,unsigned client,unsigned wave,unsigned band){
 if(client>=VC_BAND_CLIENTS||wave>=64||band>=14)return 0;vc_band_word_publish_changed(&h->desired[client],wave*14+band);return 1;
}
void vc_band_audio_release(VCBandHandoff *h,unsigned client){if(client<VC_BAND_CLIENTS)vc_band_word_publish_changed(&h->desired[client],VC_BAND_OFF);}
int vc_band_audio_begin(VCBandHandoff *h){
 unsigned sequence=vc_band_word_load(&h->published);if(sequence==h->audio_sequence)return 0;
 for(unsigned i=0;i<VC_BAND_CLIENTS;i++){h->audio_table[i]=h->proposed_table[i];h->audio_key[i]=h->proposed_key[i];}
 h->audio_sequence=sequence;
 return 1;
}
/* The voice adapter captures replacement corrections BEFORE releasing old pins. */
void vc_band_audio_end(VCBandHandoff *h){vc_band_word_publish_changed(&h->acknowledged,h->audio_sequence);}
int vc_band_audio_boundary(VCBandHandoff *h){int changed=vc_band_audio_begin(h);vc_band_audio_end(h);return changed;}
void vc_band_background(VCBandHandoff *h,unsigned budget){
 VCBandCache *c=&h->cache;
 if(h->pending){
  if(vc_band_word_load(&h->acknowledged)!=h->sequence)return;
  for(unsigned i=0;i<VC_BAND_CLIENTS;i++){int old=c->clients[i].slot;if(old>=0)c->slots[old].refs--;c->clients[i].slot=h->proposed_slot[i];}
  h->pending=0;
 }
 c->cold_clients=0;
 for(unsigned i=0;i<VC_BAND_CLIENTS;i++){if(vc_band_word_load(&h->cold[i]))c->cold_clients|=(uint64_t)1<<i;unsigned key=vc_band_word_load(&h->desired[i]);c->clients[i].wave=key==VC_BAND_OFF?-1:(int)(key/14);c->clients[i].band=key==VC_BAND_OFF?-1:(int)(key%14);}
 c->cold_fallback=1;
 int completed=vc_band_cache_service(c,budget);
 unsigned changed=0;
 for(unsigned i=0;i<VC_BAND_CLIENTS;i++){
  VCBandClient *v=&c->clients[i];int slot=v->slot;
  if(v->wave<0)slot=-1;
  else for(unsigned j=0;j<VC_BAND_SLOTS;j++)if(c->slots[j].state==2&&c->slots[j].wave==v->wave&&c->slots[j].band==v->band){slot=(int)j;break;}
  if(v->wave>=0&&(slot<0||c->slots[slot].wave!=v->wave||c->slots[slot].band!=v->band)){
   /* Prefer an exact, less harmonic version of this wave while its requested
    * band loads. Never reuse a table with excessive harmonics for this pitch. */
   int safe=-1;
   for(unsigned j=0;j<VC_BAND_SLOTS;j++)if(c->slots[j].state==2&&c->slots[j].wave==v->wave&&c->slots[j].band>=v->band&&(safe<0||c->slots[j].band<c->slots[safe].band))safe=(int)j;
   if(safe>=0)slot=safe;
   else if(completed==1&&(c->last_completed_clients&((uint64_t)1<<i))&&c->slots[c->last_completed].band>=v->band)slot=c->last_completed;
   else if(slot>=0&&c->slots[slot].band<v->band)slot=-1;
  }
  h->proposed_slot[i]=slot;changed|=slot!=v->slot;
 }
 if(!changed)return;
 for(unsigned i=0;i<VC_BAND_CLIENTS;i++){int slot=h->proposed_slot[i];h->proposed_table[i]=slot<0?0:c->tables[slot];h->proposed_key[i]=slot<0?VC_BAND_OFF:(unsigned)c->slots[slot].wave*14+(unsigned)c->slots[slot].band;if(slot>=0)c->slots[slot].refs++;}
 h->sequence++;h->pending=1;
 vc_band_word_store(&h->published,h->sequence);
}
