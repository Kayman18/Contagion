#include "band_bank.h"
#include <string.h>
#include "band_peaks.inc"
static uint32_t rd32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint32_t crc_update(uint32_t c,const uint8_t *p,unsigned n){while(n--){c^=*p++;for(unsigned j=0;j<8;j++)c=(c>>1)^(0xedb88320u&(0u-(c&1)));}return c;
}
static uint32_t crc32(const uint8_t *p,unsigned n){return ~crc_update(~0u,p,n);}
static int unpack(const uint8_t *src,unsigned bytes,uint8_t *dst,unsigned size){
 unsigned p=0,q=0;
 while(p<bytes){unsigned token=src[p++],n=token>>4;
  if(n==15){unsigned b;do{if(p==bytes)return 0;b=src[p++];n+=b;if(n>size)return 0;}while(b==255);}
  if(n>bytes-p||n>size-q)return 0;memcpy(dst+q,src+p,n);p+=n;q+=n;
  if(p==bytes)return q==size;
  if(bytes-p<2)return 0;unsigned distance=src[p]|(unsigned)src[p+1]<<8;p+=2;
  if(!distance||distance>q)return 0;n=(token&15)+4;
  if((token&15)==15){unsigned b;do{if(p==bytes)return 0;b=src[p++];n+=b;if(n>size)return 0;}while(b==255);}
  if(n>size-q)return 0;while(n--){dst[q]=dst[q-distance];q++;}
 }return q==size;
}
int vc_band_bank_init(VCBandBank *bank,const uint8_t *p,unsigned bytes){
 if(!bank)return 0;bank->data=0;bank->bytes=0;
 if(!p||bytes!=VC_BAND_BANK_BYTES||bytes<1056||memcmp(p,"VCWAV001",8)||rd32(p+8)!=1||rd32(p+12)!=64||rd32(p+16)!=bytes)return 0;
 if(crc32(p,28)!=rd32(p+28)||crc32(p+32,1024)!=rd32(p+24)||crc32(p+1056,bytes-1056)!=rd32(p+20)||crc32(p,bytes)!=VC_BAND_BANK_CRC)return 0;
 unsigned last=1056;for(unsigned w=0;w<64;w++){const uint8_t *e=p+32+w*16;unsigned off=rd32(e),n=rd32(e+4);if(off!=last||!n||off>bytes||n>bytes-off)return 0;last=off+n;}
 if(last!=bytes)return 0;bank->data=p;bank->bytes=bytes;return 1;
}
int vc_band_bank_load(void *context,unsigned wave,double coefficients[256],float *peak){
 VCBandBank *bank=context;if(!bank||!bank->data||!coefficients||!peak||wave>=64)return 0;
 const uint8_t *e=bank->data+32+wave*16;unsigned off=rd32(e),n=rd32(e+4);
 if(off>bank->bytes||n>bank->bytes-off||!unpack(bank->data+off,n,(uint8_t*)coefficients,2048)||crc32((const uint8_t*)coefficients,2048)!=rd32(e+8))return 0;
 *peak=vc_band_peaks[wave];return 1;
}
