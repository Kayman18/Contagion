#include "../experimental/band_bank.h"
#include "../experimental/band_handoff.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static VCBandHandoff h;
static float tables[VC_BAND_SLOTS][2049],reference[64][14][2049];
static double sines[2048];static unsigned char packed[90426];
static void read_file(const char *p,void *out,size_t n){FILE *f=fopen(p,"rb");assert(f&&fread(out,n,1,f)==1);fclose(f);}
int main(void){
 read_file("data/source-waves.f32",reference,sizeof reference);read_file("data/wave-sines.f64",sines,sizeof sines);read_file("data/VIROOOS-WAV.bin",packed,sizeof packed);
 VCBandBank bank;assert(vc_band_bank_init(&bank,packed,sizeof packed));vc_band_handoff_init(&h,tables,sines,vc_band_bank_load,&bank);
 unsigned publications=0,first_ready=0,checks=0;
 for(unsigned tick=0;tick<8192;tick++){
  for(unsigned c=0;c<12;c++)vc_band_audio_request(&h,c,(tick+5*c)%64,0);
  vc_band_background(&h,1024);publications+=vc_band_audio_boundary(&h);
  unsigned ready=0;for(unsigned c=0;c<12;c++)if(h.audio_key[c]!=VC_BAND_OFF){unsigned k=h.audio_key[c];assert(!memcmp(h.audio_table[c],reference[k/14][k%14],8196));ready++;checks++;}
  if(ready==12&&!first_ready)first_ready=tick+1;
  if(first_ready)assert(ready==12);
 }
 assert(first_ready&&first_ready<1024&&publications>25);
 for(unsigned c=0;c<12;c++)vc_band_audio_request(&h,c,40+c,c%14);
 unsigned settled=0;
 for(unsigned tick=0;tick<4096;tick++){
  vc_band_background(&h,4096);vc_band_audio_boundary(&h);unsigned ready=0;
  for(unsigned c=0;c<12;c++)ready+=h.audio_key[c]==(40+c)*14+c%14;
  if(ready==12){settled=tick+1;break;}
 }
 assert(settled);
 /* Interrupt a long rebuild for a new cold note's small safe band. */
 vc_band_handoff_init(&h,tables,sines,vc_band_bank_load,&bank);
 vc_band_audio_request(&h,0,1,0);vc_band_background(&h,2048);vc_band_audio_boundary(&h);vc_band_background(&h,1);
 assert(h.cache.building>=0&&h.cache.slots[h.cache.building].band==0);
 vc_band_audio_request(&h,1,2,8);vc_band_background(&h,2048);vc_band_audio_boundary(&h);
 assert(h.audio_key[1]==2*14+13&&h.cache.cancelled==1);
 /* A stolen slot with another wave also gets a fundamental first. */
 vc_band_word_store(&h.cold[0],1);vc_band_audio_request(&h,0,3,0);
 vc_band_background(&h,2048);vc_band_audio_boundary(&h);
 assert(h.audio_key[0]==3*14+13);vc_band_word_store(&h.cold[0],0);
 /* Old slots remain protected between begin and end, including worker calls. */
 vc_band_background(&h,0);const float *old=h.audio_table[0];float copy[2049];memcpy(copy,old,sizeof copy);
 vc_band_audio_request(&h,0,63,13);
 for(unsigned i=0;!h.pending&&i<1024;i++)vc_band_background(&h,8192);
 assert(h.pending);vc_band_audio_begin(&h);
 for(unsigned i=0;i<100;i++)vc_band_background(&h,8192);
 assert(!memcmp(old,copy,sizeof copy));vc_band_audio_end(&h);
 for(unsigned c=0;c<12;c++)vc_band_audio_release(&h,c);
 for(unsigned i=0;i<4;i++){vc_band_background(&h,1);vc_band_audio_boundary(&h);}
 vc_band_background(&h,0);for(unsigned i=0;i<VC_BAND_SLOTS;i++)assert(h.cache.slots[i].refs==0);
 printf("{\"scan_requests\":98304,\"exact_table_checks\":%u,\"publications\":%u,\"first_all_ready_tick\":%u,\"no_later_missing_tables\":true,\"settled_ticks\":%u,\"deferred_ack_preserves_old_bytes\":true,\"all_pins_released\":true,\"pass\":true}\n",checks,publications,first_ready,settled);
}
