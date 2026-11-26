#include "../engine/wave_cache.h"
#include "../experimental/band_voice.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
void vc_band_device_attach(VCSynth*);
void vc_band_device_service(unsigned);
static VCSynth synth;
static float reference[14][2049];
int main(void){
 assert(vc_wave_init(vc_wave_seed(),vc_wave_seed_size()));
 vc_band_device_attach(&synth);
 FILE *f=fopen("data/source-waves.f32","rb");assert(f);
 for(unsigned w=0;w<64;w++){
  assert(fread(reference,sizeof(reference),1,f)==1);
  assert(vc_wave_begin(2,w));unsigned steps=0;int result;
  do{result=vc_wave_service(1024);assert(++steps<300);}while(!result);
  assert(result==1&&vc_wave_cached(2)==(int)w);
  assert(!memcmp(reference[0],vc_wave_table(w,0),8196));
  vc_band_voice_boundary(&synth);vc_band_device_service(1024);
 }
 fclose(f);assert(!vc_wave_begin(3,0));assert(!vc_wave_begin(2,64));
 puts("{\"device_lfo_tables_exact\":64,\"pass\":true}");
}
