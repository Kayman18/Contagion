#include "../experimental/band_bank.h"
#include "../experimental/band_handoff.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
static unsigned char packed[90426];static double sines[2048];
static float tables[VC_BAND_SLOTS][2049],reference[64][14][2049];static VCBandHandoff h;
static void read_file(const char *p,void *out,size_t size){FILE *f=fopen(p,"rb");assert(f&&fread(out,size,1,f)==1);fclose(f);}
int main(void){
 read_file("data/VIROOOS-WAV.bin",packed,sizeof packed);read_file("data/wave-sines.f64",sines,sizeof sines);read_file("data/source-waves.f32",reference,sizeof reference);
 VCBandBank bank;assert(vc_band_bank_init(&bank,packed,sizeof packed));vc_band_handoff_init(&h,tables,sines,vc_band_bank_load,&bank);
 unsigned hits=0;
 for(unsigned tick=0;tick<2400;tick++){
  unsigned wave=tick%4;vc_band_audio_request(&h,0,wave,0);
  vc_band_background(&h,1024);vc_band_audio_boundary(&h);
  if(h.audio_key[0]==wave*14){hits++;assert(!memcmp(h.audio_table[0],reference[wave][0],8196));}
 }
 printf("scan completed %u cancellations %u hits %u\n",h.cache.completed,h.cache.cancelled,hits);assert(h.cache.completed>=4&&h.cache.completed<=8&&h.cache.cancelled==0&&hits>1000);
 /* Silence/release should cancel abandoned work and free old references. */
 vc_band_audio_request(&h,0,20,0);vc_band_background(&h,1);vc_band_audio_boundary(&h);vc_band_background(&h,1);
 assert(h.cache.building>=0);vc_band_audio_release(&h,0);
 for(unsigned n=0;n<3;n++){vc_band_background(&h,1);vc_band_audio_boundary(&h);}
 assert(h.cache.building<0&&h.cache.cancelled==1&&h.audio_key[0]==VC_BAND_OFF);
 for(unsigned i=0;i<VC_BAND_SLOTS;i++)assert(!h.cache.slots[i].refs);
 printf("{\"rapid_scan_requests\":2400,\"completed_exact_tables\":5,\"requested_wave_hits\":%u,\"scan_cancellations\":0,\"release_cancels_abandoned_load\":true,\"pass\":true}\n",hits);
}
