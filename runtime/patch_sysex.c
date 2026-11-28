#include "patch_sysex.h"
#include "patch_record.h"
#include <string.h>
static int length_ok(unsigned cmd,unsigned n){return (cmd==1&&n==0)||((cmd==2||cmd==3)&&n==VC_PATCH_BYTES)||(cmd==4&&n==1);}
unsigned vc_patch_sysex_encode(uint8_t *out,unsigned cap,unsigned cmd,unsigned slot,const uint8_t *raw,unsigned n){
 unsigned need=10+n+(n+6)/7;if(!out||slot>=VC_PATCH_SLOTS||!length_ok(cmd,n)||(n&&!raw)||cap<need)return 0;
 const uint8_t header[]={0xf0,0x7d,0x56,0x49,0x52,1};memcpy(out,header,6);out[6]=cmd;out[7]=slot;unsigned at=8;
 for(unsigned i=0;i<n;){unsigned mask=at++;out[mask]=0;for(unsigned b=0;b<7&&i<n;b++,i++){out[mask]|=(raw[i]>>7)<<b;out[at++]=raw[i]&127;}}
 unsigned sum=0;for(unsigned i=1;i<at;i++)sum+=out[i];out[at++]=(uint8_t)((0u-sum)&127);out[at++]=0xf7;return at;
}
/* Body only, excluding F0/F7, matching the platform callback contract. */
int vc_patch_sysex_decode(const uint8_t *p,unsigned n,unsigned *cmd,unsigned *slot,uint8_t *raw,unsigned *bytes){
 const uint8_t header[]={0x7d,0x56,0x49,0x52,1};
 if(!p||!cmd||!slot||!raw||!bytes||n<8||n>VC_PATCH_WIRE_MAX-2||memcmp(p,header,5)||p[6]>=VC_PATCH_SLOTS)return 0;
 unsigned sum=0;for(unsigned i=0;i<n;i++){if(p[i]>127)return 0;sum+=p[i];}if(sum&127)return 0;
 uint8_t tmp[VC_PATCH_BYTES];unsigned at=7,count=0;
 while(at<n-1){unsigned mask=p[at++],b=0;if(at==n-1)return 0;
  for(;b<7&&at<n-1;b++){if(count==sizeof(tmp))return 0;tmp[count++]=p[at++]|((mask>>b&1)<<7);}
  if(mask>>b)return 0;
 }
 if(!length_ok(p[5],count))return 0;
 memcpy(raw,tmp,count);*cmd=p[5];*slot=p[6];*bytes=count;return 1;
}
