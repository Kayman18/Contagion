#include "virooos.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
static void emit(float x){assert(isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 const float rates[]={22050,44100,48000};
 for(int r=0;r<3;r++)for(int color=0;color<128;color++){
  VCNoise n={0};n.rng=0x12345678;vc_noise_setup(&n,rates[r],color);
  for(int i=0;i<2048;i++)emit(vc_noise_next(&n));
 }
 VCNoise n={0};n.rng=0xabcdef01;
 for(int i=0;i<65536;i++){vc_noise_setup(&n,44100,(i/257)%128);emit(vc_noise_next(&n));}
 return 0;
}
