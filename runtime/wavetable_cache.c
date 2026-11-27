#include "wavetable_cache.h"
#include "wavetable_flash.h"
#include <string.h>
void vc_band_cache_init(VCBandCache *c,float tables[VC_BAND_SLOTS][2049],const double *sines,VCBandLoad load,void *context){
 memset(c,0,sizeof(*c));c->tables=tables;c->sines=sines;c->load=load;c->context=context;c->building=-1;for(unsigned i=0;i<3;i++)c->reserved[i]=-1;
 for(unsigned i=0;i<VC_BAND_CLIENTS;i++)c->clients[i]=(VCBandClient){-1,-1,-1};
}
static int find(const VCBandCache *c,int wave,int band,unsigned state){
 for(unsigned i=0;i<VC_BAND_SLOTS;i++)if(c->slots[i].state==state&&c->slots[i].wave==wave&&c->slots[i].band==band)return (int)i;return -1;
}
static int reserved(const VCBandCache *c,const VCBandSlot *s){
 if(s->state!=2||s->band!=13)return 0;for(unsigned i=0;i<3;i++)if(c->reserved[i]==s->wave)return 1;return 0;
}
static int wanted(const VCBandCache *c,const VCBandSlot *s){
 for(unsigned i=0;i<VC_BAND_CLIENTS;i++)if(c->clients[i].wave==s->wave&&(s->band==13||c->clients[i].band==s->band||(c->clients[i].wave==s->wave&&c->clients[i].band<=s->band)))return 1;return 0;
}
int vc_band_cache_request(VCBandCache *c,unsigned client,unsigned wave,unsigned band){
 if(client>=VC_BAND_CLIENTS||wave>=64||band>=14)return 0;
 c->clients[client].wave=(int)wave;c->clients[client].band=(int)band;return 1;
}
int vc_band_cache_commit(VCBandCache *c,unsigned client){
 if(client>=VC_BAND_CLIENTS)return 0;VCBandClient *v=&c->clients[client];if(v->wave<0)return 0;
 int slot=find(c,v->wave,v->band,2);if(slot<0)return 0;if(v->slot==slot)return 1;
 c->slots[slot].refs++;if(v->slot>=0)c->slots[v->slot].refs--;v->slot=slot;return 1;
}
void vc_band_cache_release(VCBandCache *c,unsigned client){
 if(client>=VC_BAND_CLIENTS)return;VCBandClient *v=&c->clients[client];if(v->slot>=0)c->slots[v->slot].refs--;*v=(VCBandClient){-1,-1,-1};
}
const float *vc_band_cache_current(const VCBandCache *c,unsigned client){
 if(client>=VC_BAND_CLIENTS||c->clients[client].slot<0)return 0;return c->tables[c->clients[client].slot];
}
/* A scan can change wave every control tick. Finish the current table while
 * any active client still needs this pitch band, so repeated scans warm cache.
 * If every client leaves the band, cancel rather than delay a new pitch. */
static int band_needed(const VCBandCache *c,const VCBandSlot *s){
 for(unsigned i=0;i<VC_BAND_CLIENTS;i++)if(c->clients[i].wave>=0&&(s->band==13||c->clients[i].band==s->band||(c->clients[i].wave==s->wave&&c->clients[i].band<=s->band)))return 1;
 return 0;
}
static int has_safe(const VCBandCache *c,const VCBandClient *v){
 if(v->slot>=0&&c->slots[v->slot].band>=v->band&&(!(c->cold_clients&((uint64_t)1<<(unsigned)(v-c->clients)))||c->slots[v->slot].wave==v->wave))return 1;
 for(unsigned i=0;i<VC_BAND_SLOTS;i++)if(c->slots[i].state==2&&c->slots[i].wave==v->wave&&c->slots[i].band>=v->band)return 1;
 return 0;
}
int vc_band_cache_service(VCBandCache *c,unsigned budget){
 if(!budget)return 0;
 if(c->building>=0&&!band_needed(c,&c->slots[c->building])){c->slots[c->building].state=0;c->building=-1;c->cancelled++;}
 /* A new cold note must not wait behind a full low-band scan rebuild. */
 if(c->cold_fallback&&c->building>=0&&c->slots[c->building].band<13){
  for(unsigned i=0;i<VC_BAND_CLIENTS;i++)if(c->clients[i].wave>=0&&!has_safe(c,&c->clients[i])){
   c->slots[c->building].state=0;c->building=-1;c->cancelled++;break;
  }
 }
 if(c->building<0){
  int client=-1;
  /* Serve clients without a usable pitch band before same-band wave scans.
   * Keep round-robin ordering within each class. Never interrupt a build. */
  for(unsigned priority=0;priority<2&&client<0;priority++)for(unsigned n=0;n<VC_BAND_CLIENTS;n++){
   unsigned i=(c->next_client+n)%VC_BAND_CLIENTS;VCBandClient *v=&c->clients[i];
   unsigned urgent=((c->cold_clients&((uint64_t)1<<i))&&!has_safe(c,v))||v->slot<0||(v->band>=0&&c->slots[v->slot].band!=v->band);
   if(v->wave>=0&&urgent==(priority==0)&&find(c,v->wave,v->band,2)<0){client=(int)i;c->next_client=(i+1)%VC_BAND_CLIENTS;break;}
  }
  if(client<0)return 0;
  int slot=-1;
  for(unsigned n=0;n<VC_BAND_SLOTS;n++){unsigned i=(c->next_slot+n)%VC_BAND_SLOTS;VCBandSlot *s=&c->slots[i];if(!s->refs&&!reserved(c,s)&&(s->state==0||!wanted(c,s))){slot=(int)i;c->next_slot=(i+1)%VC_BAND_SLOTS;break;}}
  if(slot<0)return 0;
  VCBandClient *v=&c->clients[client];VCBandSlot *s=&c->slots[slot];float peak;
  unsigned band=(unsigned)v->band;
  /* A cold voice gets its exact fundamental band first (2048 operations),
   * then the full requested band. Existing safe audio is never interrupted. */
  int usable=has_safe(c,v);
  if(c->cold_fallback&&!usable)band=13;
  s->state=0;
  c->building_clients=0;for(unsigned i=0;i<VC_BAND_CLIENTS;i++)if(c->clients[i].wave==v->wave&&c->clients[i].band==v->band)c->building_clients|=(uint64_t)1<<i;
  if(vc_wave_flash_copy((unsigned)v->wave,band,c->tables[slot])){
   *s=(VCBandSlot){v->wave,(int)band,2,0};
   c->last_completed=slot;c->last_completed_clients=c->building_clients;c->completed++;return 1;
  }
  if(!c->load||!c->load(c->context,(unsigned)v->wave,c->coefficients,&peak)||!vc_band_begin(&c->builder,c->coefficients,c->sines,peak,band,c->tables[slot])){c->failures++;return -1;}
  *s=(VCBandSlot){v->wave,(int)band,1,0};c->building=slot;
 }
 if(vc_band_service(&c->builder,budget)==1){c->slots[c->building].state=2;c->last_completed=c->building;c->last_completed_clients=c->building_clients;c->building=-1;c->completed++;return 1;}return 0;
}
