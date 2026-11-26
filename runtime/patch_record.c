#include "patch_record.h"
#include "patch_schema.inc"
#include <string.h>
static unsigned read32(const uint8_t *p){return (unsigned)p[0]|(unsigned)p[1]<<8|(unsigned)p[2]<<16|(unsigned)p[3]<<24;}
static void write32(uint8_t *p,unsigned x){for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(x>>(8*i));}
static unsigned crc(const uint8_t *p,unsigned n){unsigned c=~0u;while(n--){c^=*p++;for(unsigned i=0;i<8;i++)c=(c>>1)^(0xedb88320u&(0u-(c&1)));}return ~c;}
unsigned vc_patch_schema(void){return VC_PATCH_SCHEMA;}
static int valid(const VCPatchRecord *r){
 if(!r||r->name[10])return 0;
 unsigned ended=0;for(unsigned i=0;i<10;i++){unsigned c=(unsigned char)r->name[i];if(!c)ended=1;else if(ended||c<32||c>126)return 0;}
 for(unsigned i=0;i<VC_PARAMS;i++)if(r->params[i]>vc_param_max(i))return 0;
 for(unsigned i=0;i<VC_MATRIX_ROUTES;i++){const VCMatrixRoute *x=&r->matrix.route[i];if(x->source>=VC_MATRIX_SOURCES||x->destination>=VC_MATRIX_DESTINATIONS||x->amount>127)return 0;}
 return 1;
}
int vc_patch_encode(uint8_t *out,unsigned bytes,const VCPatchRecord *r){
 if(!out||bytes<VC_PATCH_BYTES||!valid(r))return 0;
 memcpy(out,"VCP1",4);write32(out+4,VC_PATCH_SCHEMA);write32(out+8,VC_PARAMS);write32(out+12,VC_PATCH_BYTES);
 unsigned at=16;memcpy(out+at,r->name,10);at+=10;memcpy(out+at,r->params,VC_PARAMS);at+=VC_PARAMS;
 for(unsigned i=0;i<VC_MATRIX_ROUTES;i++){const VCMatrixRoute *x=&r->matrix.route[i];out[at++]=x->source;out[at++]=x->destination;out[at++]=x->amount;}
 write32(out+at,crc(out,at));return 1;
}
int vc_patch_decode(VCPatchRecord *r,const uint8_t *in,unsigned bytes){
 if(!r||!in||bytes!=VC_PATCH_BYTES||memcmp(in,"VCP1",4)||read32(in+4)!=VC_PATCH_SCHEMA||read32(in+8)!=VC_PARAMS||read32(in+12)!=VC_PATCH_BYTES||read32(in+bytes-4)!=crc(in,bytes-4))return 0;
 VCPatchRecord tmp;memset(&tmp,0,sizeof(tmp));unsigned at=16;memcpy(tmp.name,in+at,10);at+=10;memcpy(tmp.params,in+at,VC_PARAMS);at+=VC_PARAMS;
 for(unsigned i=0;i<VC_MATRIX_ROUTES;i++){VCMatrixRoute *x=&tmp.matrix.route[i];x->source=in[at++];x->destination=in[at++];x->amount=in[at++];}
 if(!valid(&tmp))return 0;*r=tmp;return 1;
}

void vc_patch_bank_init(uint8_t bank[VC_PATCH_BANK_BYTES]){
 memset(bank,0,VC_PATCH_BANK_BYTES);memcpy(bank,"VCB1",4);
 write32(bank+4,VC_PATCH_SCHEMA);write32(bank+8,VC_PATCH_BANK_BYTES);
 write32(bank+VC_PATCH_BANK_BYTES-4,crc(bank,VC_PATCH_BANK_BYTES-4));
}
int vc_patch_bank_valid(const uint8_t *bank,unsigned bytes){
 if(!bank||bytes!=VC_PATCH_BANK_BYTES||memcmp(bank,"VCB1",4)||read32(bank+4)!=VC_PATCH_SCHEMA||read32(bank+8)!=bytes||read32(bank+12)>65535||read32(bank+bytes-4)!=crc(bank,bytes-4))return 0;
 VCPatchRecord tmp;unsigned mask=read32(bank+12);
 for(unsigned i=0;i<16;i++)if(mask&(1u<<i))if(!vc_patch_decode(&tmp,bank+16+i*VC_PATCH_BYTES,VC_PATCH_BYTES))return 0;
 return 1;
}
int vc_patch_bank_put(uint8_t *bank,unsigned bytes,unsigned slot,const VCPatchRecord *record){
 uint8_t encoded[VC_PATCH_BYTES];
 if(slot>=16||!vc_patch_bank_valid(bank,bytes)||!vc_patch_encode(encoded,sizeof(encoded),record))return 0;
 memcpy(bank+16+slot*VC_PATCH_BYTES,encoded,sizeof(encoded));write32(bank+12,read32(bank+12)|(1u<<slot));write32(bank+bytes-4,crc(bank,bytes-4));return 1;
}
int vc_patch_bank_get(VCPatchRecord *record,const uint8_t *bank,unsigned bytes,unsigned slot){
 if(slot>=16||!vc_patch_bank_valid(bank,bytes)||!(read32(bank+12)&(1u<<slot)))return 0;
 return vc_patch_decode(record,bank+16+slot*VC_PATCH_BYTES,VC_PATCH_BYTES);
}
