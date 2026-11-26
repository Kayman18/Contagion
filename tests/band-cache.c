#include "../experimental/band_cache.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static VCBandCache cache;
static float tables[VC_BAND_SLOTS][2049],reference[64][14][2049],peaks[64];
static double coefficients[64][256],sines[2048];
static unsigned fail_load,checks;
static int load(void *ctx,unsigned wave,double out[256],float *peak){(void)ctx;if(fail_load)return 0;memcpy(out,coefficients[wave],2048);*peak=peaks[wave];return 1;}
static void read_file(const char *path,void *data,size_t size){FILE *f=fopen(path,"rb");assert(f&&fread(data,size,1,f)==1);fclose(f);}
static void verify(void){
 unsigned refs[VC_BAND_SLOTS]={0};for(unsigned i=0;i<12;i++){int s=cache.clients[i].slot;if(s>=0){refs[s]++;VCBandSlot *slot=&cache.slots[s];assert(slot->state==2);assert(!memcmp(vc_band_cache_current(&cache,i),reference[slot->wave][slot->band],8196));}}
 for(unsigned i=0;i<VC_BAND_SLOTS;i++){assert(refs[i]==cache.slots[i].refs);if(cache.slots[i].state==1)assert(!refs[i]);}checks++;
}
static void settle(void){for(unsigned n=0;n<5000;n++){vc_band_cache_service(&cache,4096);verify();unsigned ready=0;for(unsigned i=0;i<12;i++)ready+=cache.clients[i].wave<0||vc_band_cache_commit(&cache,i);verify();if(ready==12)return;}assert(!"cache failed to settle");}
int main(void){
 read_file("data/source-waves.f32",reference,sizeof(reference));read_file("data/wave-coefficients.f64",coefficients,sizeof(coefficients));read_file("data/wave-sines.f64",sines,sizeof(sines));read_file("experimental/wave-peaks.f32",peaks,sizeof(peaks));vc_band_cache_init(&cache,tables,sines,load,0);
 /* A new client gets a table before an old client's same-band wave scan. */
 vc_band_cache_request(&cache,0,0,13);vc_band_cache_service(&cache,4096);assert(vc_band_cache_commit(&cache,0));
 vc_band_cache_request(&cache,0,1,13);vc_band_cache_request(&cache,1,2,13);cache.next_client=0;
 vc_band_cache_service(&cache,4096);assert(vc_band_cache_commit(&cache,1));assert(!vc_band_cache_commit(&cache,0));verify();
 vc_band_cache_service(&cache,4096);assert(vc_band_cache_commit(&cache,0));
 /* A pitch-band miss also precedes a same-band scan even with an old slot. */
 vc_band_cache_request(&cache,0,3,13);vc_band_cache_request(&cache,1,2,12);cache.next_client=0;
 vc_band_cache_service(&cache,8192);assert(vc_band_cache_commit(&cache,1));assert(!vc_band_cache_commit(&cache,0));verify();
 vc_band_cache_init(&cache,tables,sines,load,0);
 assert(!vc_band_cache_request(&cache,12,0,0)&&!vc_band_cache_request(&cache,0,64,0)&&!vc_band_cache_request(&cache,0,0,14));
 for(unsigned i=0;i<12;i++)assert(vc_band_cache_request(&cache,i,i,0));settle();
 for(unsigned i=0;i<12;i++)assert(vc_band_cache_request(&cache,i,20+i,13));
 /* Pinned old bands survive service; no implicit publication to clients. */
 for(unsigned n=0;n<10;n++){vc_band_cache_service(&cache,4096);verify();}settle();
 for(unsigned i=0;i<12;i++)vc_band_cache_request(&cache,i,50,7);settle();int shared=cache.clients[0].slot;for(unsigned i=1;i<12;i++)assert(cache.clients[i].slot==shared);assert(cache.slots[shared].refs==12);
 vc_band_cache_request(&cache,0,60,0);vc_band_cache_service(&cache,1);assert(cache.building>=0);vc_band_cache_request(&cache,0,61,13);settle();assert(cache.cancelled>0);
 fail_load=1;vc_band_cache_request(&cache,0,62,12);assert(vc_band_cache_service(&cache,100)==-1);verify();fail_load=0;settle();
 unsigned rng=1234567;
 for(unsigned n=0;n<3000;n++){rng=rng*1664525u+1013904223u;unsigned client=(rng>>24)%12;if(n%11==0)vc_band_cache_release(&cache,client);else vc_band_cache_request(&cache,client,(rng>>8)%64,(rng>>16)%14);vc_band_cache_service(&cache,1+(rng%8192));verify();for(unsigned i=0;i<12;i++)vc_band_cache_commit(&cache,i);verify();}settle();
 for(unsigned i=0;i<12;i++)vc_band_cache_release(&cache,i);verify();
 printf("{\"invariant_checks\":%u,\"completed_builds\":%u,\"cancelled_builds\":%u,\"failure_recovery\":true,\"shared_references\":true,\"pinned_tables_exact\":true,\"rapid_changes\":3000,\"slots\":32,\"clients\":12,\"table_bytes\":%zu,\"host_metadata_and_builder_bytes\":%zu,\"serialized_only\":true,\"pass\":true}\n",checks,cache.completed,cache.cancelled,sizeof(tables),sizeof(cache));
}
