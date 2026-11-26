#include "band_rebuild.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
static double coefficients[64][256],sines[2048];static float peaks[64],out[2049];
static void read_file(const char *p,void *v,size_t n){FILE *f=fopen(p,"rb");assert(f&&fread(v,n,1,f)==1);fclose(f);}
int main(void){read_file("data/wave-coefficients.f64",coefficients,sizeof coefficients);read_file("data/wave-sines.f64",sines,sizeof sines);read_file("experimental/wave-peaks.f32",peaks,sizeof peaks);
 clock_t start=clock();double sum=0;
 for(unsigned repeat=0;repeat<3;repeat++)for(unsigned w=0;w<64;w++)for(unsigned b=0;b<14;b++){
  VCBandRebuild state;assert(vc_band_begin(&state,coefficients[w],sines,peaks[w],b,out));while(!vc_band_service(&state,1024)){}sum+=out[100];
 }
 printf("{\"host_cpu_seconds\":%.6f,\"bands\":2688,\"checksum\":%.17g,\"hardware_timing\":false}\n",(double)(clock()-start)/CLOCKS_PER_SEC,sum);
}
