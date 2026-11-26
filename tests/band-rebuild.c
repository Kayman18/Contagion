#include "../experimental/band_rebuild.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
static double c[256],sines[2048];
static float expected[2049],out[2049],again[2049],peaks[64];
static const unsigned hs[14]={128,96,64,48,32,24,16,12,8,6,4,3,2,1};
int main(void){
 FILE *cf=fopen("data/wave-coefficients.f64","rb"),*sf=fopen("data/wave-sines.f64","rb"),*wf=fopen("data/source-waves.f32","rb");assert(cf&&sf&&wf);assert(fread(sines,sizeof(sines),1,sf)==1);fclose(sf);
 unsigned bands=0;unsigned long total=0;
 for(unsigned wave=0;wave<64;wave++){
  assert(fread(c,sizeof(c),1,cf)==1);float peak=0;
  for(unsigned n=0;n<2048;n++){double sum=0;for(unsigned h=1;h<=128;h++){unsigned j=h*n&2047;double a=c[h-1],b=c[128+h-1];if(a!=0||b!=0)sum+=a*sines[j]+b*sines[(j+512)&2047];}float value=fabsf((float)sum);if(value>peak)peak=value;}
  peaks[wave]=peak;
  for(unsigned band=0;band<14;band++){
   assert(fread(expected,sizeof(expected),1,wf)==1);VCBandRebuild state={0};assert(vc_band_begin(&state,c,sines,peak,band,out));unsigned budgets[]={1,17,128,4096};unsigned budget=budgets[(wave+band)%4];int status;do{unsigned long before=state.operations;status=vc_band_service(&state,budget);assert(state.operations-before<=budget);}while(!status);assert(status==1);assert(state.operations==2048ul*hs[band]);assert(!memcmp(out,expected,sizeof(out)));total+=state.operations;bands++;
   if(wave==0||wave==63){assert(vc_band_begin(&state,c,sines,peak,band,again));assert(vc_band_service(&state,0)==0);while(!vc_band_service(&state,37)){}assert(!memcmp(out,again,sizeof(out)));}
  }
 }
 fclose(cf);fclose(wf);VCBandRebuild state={0};assert(!vc_band_begin(&state,c,sines,1,14,out));assert(!vc_band_begin(&state,c,sines,NAN,0,out));assert(vc_band_service(&state,1)==-1);
 FILE *pf=fopen("experimental/wave-peaks.f32","wb");assert(pf&&fwrite(peaks,sizeof(peaks),1,pf)==1);fclose(pf);
 printf("{\"bands\":%u,\"samples\":%u,\"byte_identical\":true,\"budget_invariant\":true,\"harmonic_steps\":%lu,\"min_steps_per_band\":2048,\"max_steps_per_band\":262144,\"band_bytes\":8196,\"coefficient_bytes\":2048,\"peak_table_bytes\":256,\"host_state_bytes\":%zu,\"pass\":true}\n",bands,bands*2049,total,sizeof(VCBandRebuild));
}
