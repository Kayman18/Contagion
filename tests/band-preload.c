/* Include backend to inspect cache ownership at the muted/render boundary. */
#include "../experimental/band_device.c"
#include <assert.h>
#include <stdio.h>
static VCSynth synth;
static unsigned last_calls;
static float reference[64][14][2049];
static void prepare(unsigned a,unsigned b,unsigned c){
 unsigned waves[3]={a,b,c},calls=0;int result;
 do{result=vc_band_device_prepare(waves,4096);assert(++calls<1200);}while(!result);
 last_calls=calls;
 assert(result==1&&binding.synth==&synth&&!handoff.pending);
 for(unsigned w=0;w<3;w++)for(unsigned band=0;band<14;band++){
  if(band>=5&&band!=13)continue;
  int found=0;for(unsigned i=0;i<VC_BAND_SLOTS;i++){
   VCBandSlot *s=&handoff.cache.slots[i];assert(!s->refs);
   if(s->state==2&&s->wave==(int)waves[w]&&s->band==(int)band){assert(!memcmp(tables[i],reference[waves[w]][band],8196));found++;}
  }assert(found==1);
 }
 unsigned stats[6];vc_band_device_stats(stats);assert(stats[0]>=(a==b&&b==c?6:18)&&stats[0]<=VC_BAND_SLOTS&&stats[1]==0&&stats[4]==0&&stats[5]==100);
 /* First requested prepared band is published without any reconstruction. */
 unsigned completed=handoff.cache.completed;
 vc_band_audio_request(&handoff,0,a,2);vc_band_device_service(1);vc_band_audio_boundary(&handoff);
 assert(handoff.audio_key[0]==a*14+2&&handoff.cache.completed==completed);
 vc_band_device_service(0);vc_band_device_stats(stats);assert(stats[1]==0);
}
int main(void){
 FILE *f=fopen("data/source-waves.f32","rb");assert(f&&fread(reference,sizeof(reference),1,f)==1);fclose(f);
 assert(vc_wave_init(vc_wave_seed(),vc_wave_seed_size()));vc_band_device_attach(&synth);
 prepare(0,1,2);prepare(0,1,2);assert(handoff.cache.completed==0&&last_calls==1);prepare(0,1,3);assert(handoff.cache.completed==6);prepare(3,3,3);assert(handoff.cache.completed==0&&last_calls==1);prepare(17,32,63);
 unsigned interrupted[3]={4,5,6};assert(!vc_band_device_prepare(interrupted,1));prepare(7,8,9);
 assert(vc_wave_init(vc_wave_seed(),vc_wave_seed_size()));
 assert(!vc_band_device_prepare(interrupted,1));
 assert(vc_wave_init(vc_wave_seed(),vc_wave_seed_size()));prepare(4,5,6);
 puts("{\"distinct_and_duplicate_waves\":true,\"all_preloaded_bands_exact\":true,\"first_request_needs_no_rebuild\":true,\"interrupted_selection_recovers\":true,\"pending_publication_reset_while_muted\":true,\"identical_selection_zero_rebuilds\":true,\"one_changed_wave_six_rebuilds\":true,\"bank_reinit_restarts_preload\":true,\"pass\":true}");
}
