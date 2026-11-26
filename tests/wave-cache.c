#include "wave_cache.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void){
 unsigned n=vc_wave_seed_size();unsigned char *bad=malloc(n);assert(bad);
 memcpy(bad,vc_wave_seed(),n);bad[n-1]^=1;assert(!vc_wave_init(bad,n));
 assert(!vc_wave_init(vc_wave_seed(),n-1));assert(vc_wave_init(vc_wave_seed(),n));
 assert(!vc_wave_begin(3,0)&&!vc_wave_begin(0,64));
 for(unsigned w=0;w<64;w++){
  assert(vc_wave_begin(2,w));assert(vc_wave_cached(2)==-1);
  int status=0;unsigned calls=0,progress=0;while(!status){status=vc_wave_service(257);unsigned next=vc_wave_progress();assert(next>=progress&&next<=100);progress=next;assert(++calls<2000);}
  assert(status==1&&vc_wave_cached(2)==(int)w);
  for(unsigned b=0;b<14;b++)assert(fwrite(vc_wave_table(w,b),4,2049,stdout)==2049);
 }
 /* Keep three different waves resident; rebuilding one must not touch others. */
 float *saved=malloc(2*14*2049*sizeof(float));assert(saved);
 for(unsigned slot=0;slot<3;slot++){
  assert(vc_wave_begin(slot,slot==0?0:slot==1?31:63));while(vc_wave_service(4096)==0){}
  assert(vc_wave_cached(slot)==(int)(slot==0?0:slot==1?31:63));
 }
 for(unsigned slot=0;slot<2;slot++)for(unsigned b=0;b<14;b++)memcpy(saved+(slot*14+b)*2049,vc_wave_table(slot?31:0,b),2049*sizeof(float));
 assert(vc_wave_begin(2,62));while(vc_wave_service(4096)==0){}assert(vc_wave_cached(2)==62);
 for(unsigned slot=0;slot<2;slot++)for(unsigned b=0;b<14;b++)assert(!memcmp(saved+(slot*14+b)*2049,vc_wave_table(slot?31:0,b),2049*sizeof(float)));
 free(saved);free(bad);return 0;
}
