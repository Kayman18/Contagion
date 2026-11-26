#include "../experimental/band_handoff.h"
#include "../experimental/band_bank.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static VCBandHandoff h;static float tables[VC_BAND_SLOTS][2049],reference[64][14][2049];static double sines[2048],expected[64][256];static uint8_t packed[90426];
static void read_file(const char *p,void *data,size_t n){FILE *f=fopen(p,"rb");assert(f&&fread(data,n,1,f)==1);fclose(f);}
int main(void){
 read_file("data/VIROOOS-WAV.bin",packed,sizeof(packed));read_file("data/source-waves.f32",reference,sizeof(reference));read_file("data/wave-coefficients.f64",expected,sizeof(expected));read_file("data/wave-sines.f64",sines,sizeof(sines));VCBandBank bank;assert(vc_band_bank_init(&bank,packed,sizeof(packed)));
 double decoded[256];float peak;for(unsigned w=0;w<64;w++){assert(vc_band_bank_load(&bank,w,decoded,&peak));assert(!memcmp(decoded,expected[w],2048));}assert(!vc_band_bank_load(&bank,64,decoded,&peak));
 for(unsigned n=0;n<1056;n+=31)assert(!vc_band_bank_init(&bank,packed,n));
 const unsigned corrupt[]={0,28,32,1056,90425};for(unsigned i=0;i<5;i++){packed[corrupt[i]]^=1;assert(!vc_band_bank_init(&bank,packed,sizeof(packed)));packed[corrupt[i]]^=1;}assert(vc_band_bank_init(&bank,packed,sizeof(packed)));
 vc_band_handoff_init(&h,tables,sines,vc_band_bank_load,&bank);unsigned bands=0;
 for(unsigned w=0;w<64;w++)for(unsigned b=0;b<14;b++){
  vc_band_audio_request(&h,0,w,b);unsigned count=0;
  do{vc_band_background(&h,4096);vc_band_audio_boundary(&h);assert(++count<1000);}while(h.audio_key[0]!=w*14+b);
  assert(!memcmp(h.audio_table[0],reference[w][b],8196));bands++;
 }
 /* A corrupted compressed block must not be published over the playing band. */
 const float *old=h.audio_table[0];unsigned oldkey=h.audio_key[0];vc_band_audio_request(&h,0,0,0);packed[1056]^=1;vc_band_background(&h,4096);vc_band_audio_boundary(&h);assert(h.audio_table[0]==old&&h.audio_key[0]==oldkey&&h.cache.failures);packed[1056]^=1;
 for(unsigned i=0;i<1000&&h.audio_key[0]!=0;i++){vc_band_background(&h,4096);vc_band_audio_boundary(&h);}assert(h.audio_key[0]==0&&!memcmp(h.audio_table[0],reference[0][0],8196));
 printf("{\"coefficient_blocks_exact\":64,\"bands_published_exact\":%u,\"samples\":%u,\"truncated_banks_rejected\":true,\"corruption_rejected\":true,\"failed_load_keeps_playing_table\":true,\"recovery\":true,\"bank_bytes\":90426,\"pass\":true}\n",bands,bands*2049);
}
