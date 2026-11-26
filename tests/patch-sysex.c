#include "../experimental/patch_sysex.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){uint8_t raw[189],out[189],frame[226];for(unsigned i=0;i<189;i++)raw[i]=(uint8_t)i;
 unsigned n=vc_patch_sysex_encode(frame,sizeof(frame),3,15,raw,sizeof(raw));assert(n==226);
 unsigned cmd=99,slot=99,bytes=99;assert(vc_patch_sysex_decode(frame+1,n-2,&cmd,&slot,out,&bytes)&&cmd==3&&slot==15&&bytes==189&&!memcmp(raw,out,189));
 unsigned corruptions=0;for(unsigned i=1;i<n-1;i++)for(unsigned bit=0;bit<8;bit++){
  frame[i]^=1u<<bit;cmd=slot=bytes=99;assert(!vc_patch_sysex_decode(frame+1,n-2,&cmd,&slot,out,&bytes)&&cmd==99&&slot==99&&bytes==99);frame[i]^=1u<<bit;corruptions++;
 }
 for(unsigned len=0;len<n-2;len++)assert(!vc_patch_sysex_decode(frame+1,len,&cmd,&slot,out,&bytes));
 assert(!vc_patch_sysex_encode(frame,sizeof(frame),3,16,raw,189));assert(!vc_patch_sysex_encode(frame,225,3,0,raw,189));
 printf("{\"wire_bytes\":226,\"single_bit_corruptions_rejected\":%u,\"truncations_rejected\":true,\"pass\":true}\n",corruptions);
}
