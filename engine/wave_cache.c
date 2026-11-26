/* Original Le reconstruction, performed only while audio is muted. Float64
 * accumulation/normalisation preserves the captured JS's Float32 table bytes.
 * No allocation, trigonometry, decompression or reconstruction in playback. */
#include "wave_cache.h"
#include <string.h>
#ifdef PLAT_HOST
#define VC_POOL
#else
#define VC_POOL __attribute__((section(".pool")))
#endif
#include "wave_bank.inc"
static float cache[2][14][2049] VC_POOL;
/* Split the third cache across RAM regions; audio still reads one contiguous
 * 2049-float band. Main-loop reconstruction scratch belongs in normal RAM. */
static float cache3_low[4][2049];
static float cache3_high[10][2049] VC_POOL;
/* Reconstruct 64 samples at a time, preserving each sample's harmonic
 * addition order. Output tables and playback accesses stay unchanged. */
#define VC_WAVE_TILE 64u
static double accumulator[VC_WAVE_TILE];
static float *const cache_views[3][14]={
{cache[0][0],cache[0][1],cache[0][2],cache[0][3],cache[0][4],cache[0][5],cache[0][6],cache[0][7],cache[0][8],cache[0][9],cache[0][10],cache[0][11],cache[0][12],cache[0][13]},
{cache[1][0],cache[1][1],cache[1][2],cache[1][3],cache[1][4],cache[1][5],cache[1][6],cache[1][7],cache[1][8],cache[1][9],cache[1][10],cache[1][11],cache[1][12],cache[1][13]},
{cache3_low[0],cache3_low[1],cache3_low[2],cache3_low[3],cache3_high[0],cache3_high[1],cache3_high[2],cache3_high[3],cache3_high[4],cache3_high[5],cache3_high[6],cache3_high[7],cache3_high[8],cache3_high[9]}};
static const float *const cache_read[3][14]={
{cache[0][0],cache[0][1],cache[0][2],cache[0][3],cache[0][4],cache[0][5],cache[0][6],cache[0][7],cache[0][8],cache[0][9],cache[0][10],cache[0][11],cache[0][12],cache[0][13]},
{cache[1][0],cache[1][1],cache[1][2],cache[1][3],cache[1][4],cache[1][5],cache[1][6],cache[1][7],cache[1][8],cache[1][9],cache[1][10],cache[1][11],cache[1][12],cache[1][13]},
{cache3_low[0],cache3_low[1],cache3_low[2],cache3_low[3],cache3_high[0],cache3_high[1],cache3_high[2],cache3_high[3],cache3_high[4],cache3_high[5],cache3_high[6],cache3_high[7],cache3_high[8],cache3_high[9]}};
static double coefficients[256];
static const float silence[2049]={0};
static const uint8_t *bank;
static unsigned bank_bytes,slot,wave,phase,index_,harmonic,band_index,tile;
static double peak;
static int cached[3]={-1,-1,-1};
static const unsigned bands[14]={128,96,64,48,32,24,16,12,8,6,4,3,2,1};
static uint32_t rd32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint32_t crc_update(uint32_t c,const uint8_t *p,unsigned n){while(n--){c^=*p++;for(unsigned j=0;j<8;j++)c=(c>>1)^(0xedb88320u&(0u-(c&1)));}return c;
}
static uint32_t crc32(const uint8_t *p,unsigned n){return ~crc_update(~0u,p,n);}
static uint32_t cache_crc(unsigned s){uint32_t c=~0u;for(unsigned b=0;b<14;b++)c=crc_update(c,(const uint8_t *)cache_views[s][b],2049*sizeof(float));return ~c;}
/* Bounded standard LZ4 block decoder; all accesses checked before consumption. */
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
const uint8_t *vc_wave_seed(void){return vc_bank_seed;}
unsigned vc_wave_seed_size(void){return sizeof(vc_bank_seed);}
int vc_wave_bank_valid(const uint8_t *p,unsigned bytes){
 if(!p||bytes<1056||memcmp(p,"VCWAV001",8)||rd32(p+8)!=1||rd32(p+12)!=64||rd32(p+16)!=bytes)return 0;
 if(crc32(p,28)!=rd32(p+28)||crc32(p+32,1024)!=rd32(p+24)||crc32(p+1056,bytes-1056)!=rd32(p+20))return 0;
 unsigned last=1056;for(unsigned w=0;w<64;w++){const uint8_t *e=p+32+w*16;unsigned off=rd32(e),n=rd32(e+4);if(off!=last||!n||off>bytes||n>bytes-off)return 0;last=off+n;}
 return last==bytes;
}
int vc_wave_init(const uint8_t *p,unsigned bytes){
 bank=0;phase=0;cached[0]=cached[1]=cached[2]=-1;
 if(!vc_wave_bank_valid(p,bytes))return 0;bank=p;bank_bytes=bytes;return 1;
}
int vc_wave_begin(unsigned s,unsigned w){
 if(!bank||s>2||w>63)return 0;
 const uint8_t *e=bank+32+w*16;unsigned off=rd32(e),n=rd32(e+4);
 if(off>bank_bytes||n>bank_bytes-off||!unpack(bank+off,n,(uint8_t *)coefficients,sizeof(coefficients))||crc32((const uint8_t *)coefficients,sizeof(coefficients))!=rd32(e+8))return 0;
 slot=s;wave=w;cached[s]=-1;phase=1;index_=0;harmonic=1;band_index=13;tile=0;peak=0;return 1;
}
int vc_wave_service(unsigned budget){
 if(!bank)return -1;
 while(budget--&&phase){
  if(phase==1){accumulator[index_++]=0;if(index_==VC_WAVE_TILE){index_=0;phase=2;}}
  else if(phase==2){
   double a=coefficients[harmonic-1],b=coefficients[128+harmonic-1];
   if(a!=0||b!=0){unsigned j=(harmonic*(tile+index_))&2047;accumulator[index_]+=a*vc_bank_sines[j]+b*vc_bank_sines[(j+512)&2047];index_++;}else index_=VC_WAVE_TILE;
   if(index_==VC_WAVE_TILE){index_=0;if(harmonic==bands[band_index])phase=3;else harmonic++;}
  }else if(phase==3){
   float x=(float)accumulator[index_];cache_views[slot][band_index][tile+index_++]=x;
   if(band_index==0){double a=x<0?-(double)x:(double)x;if(a>peak)peak=a;}
   if(index_==VC_WAVE_TILE){index_=0;if(band_index){band_index--;harmonic++;phase=2;}
    else if((tile+=VC_WAVE_TILE)<2048){band_index=13;harmonic=1;phase=1;}
    else{for(unsigned b=0;b<14;b++)cache_views[slot][b][2048]=cache_views[slot][b][0];phase=4;}}
  }else if(phase==4){
   if(peak>0)cache_views[slot][band_index][index_]=(float)((double)cache_views[slot][band_index][index_]/peak);
   if(++index_==2049){index_=0;if(++band_index==14){
    /* CRC is checked before publishing a cache. */
    if(cache_crc(slot)!=rd32(bank+32+wave*16+12)){phase=0;bank=0;return -1;}
    cached[slot]=(int)wave;phase=0;
   }}
  }
 }return phase?0:1;
}
int vc_wave_cached(unsigned s){return s<3?cached[s]:-1;}
const float *const *vc_wave_tables(unsigned w){for(unsigned s=0;s<3;s++)if(cached[s]==(int)w)return cache_read[s];return 0;}
const float *vc_wave_table(unsigned w,unsigned b){if(b>13)return silence;const float *const *tables=vc_wave_tables(w);return tables?tables[b]:silence;}
unsigned vc_wave_progress(void){return phase==1?tile*75/2048:phase==2||phase==3?(tile*128+harmonic*VC_WAVE_TILE)*75/(2048*128):phase==4?75+band_index*25/14:100;}

uint32_t vc_wave_bytes_crc(const void *p,unsigned n){return crc32(p,n);}
