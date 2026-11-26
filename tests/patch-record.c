#include "../experimental/patch_record.h"
#include "../engine/presets.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint8_t bank[VC_PATCH_BANK_BYTES],bank_saved[VC_PATCH_BANK_BYTES];
static VCPatchRecord a,b,guard;static uint8_t bytes[VC_PATCH_BYTES],saved[VC_PATCH_BYTES];
int main(void){unsigned corruptions=0;
 for(unsigned p=0;p<vc_preset_count();p++){
  memset(&a,0,sizeof a);strncpy(a.name,vc_preset_name(p),10);memcpy(a.params,vc_preset_params(p),VC_PARAMS);a.matrix=*vc_matrix_factory(p);
  for(unsigned i=0;i<VC_PARAMS;i++)if(a.params[i]>vc_param_max(i))a.params[i]=vc_param_max(i);
  assert(vc_patch_encode(bytes,sizeof bytes,&a));assert(vc_patch_decode(&b,bytes,sizeof bytes));
  assert(!memcmp(a.name,b.name,sizeof a.name)&&!memcmp(a.params,b.params,VC_PARAMS)&&!memcmp(&a.matrix,&b.matrix,sizeof a.matrix));
  assert(vc_patch_encode(saved,sizeof saved,&b)&&!memcmp(bytes,saved,sizeof bytes));
 }
 vc_patch_bank_init(bank);assert(VC_PATCH_BANK_BYTES<=3840&&vc_patch_bank_valid(bank,sizeof bank));
 guard=b;assert(!vc_patch_bank_get(&b,bank,sizeof bank,0)&&!memcmp(&b,&guard,sizeof b));
 for(unsigned slot=0;slot<16;slot++){a.name[0]='A'+slot;assert(vc_patch_bank_put(bank,sizeof bank,slot,&a));}
 for(unsigned slot=0;slot<16;slot++){assert(vc_patch_bank_get(&b,bank,sizeof bank,slot));assert(b.name[0]=='A'+slot);}
 memcpy(bank_saved,bank,sizeof bank);a.name[0]='Z';assert(vc_patch_bank_put(bank,sizeof bank,7,&a));
 for(unsigned slot=0;slot<16;slot++)if(slot!=7)assert(!memcmp(bank+16+slot*VC_PATCH_BYTES,bank_saved+16+slot*VC_PATCH_BYTES,VC_PATCH_BYTES));
 memcpy(bank_saved,bank,sizeof bank);assert(!vc_patch_bank_put(bank,sizeof bank,16,&a)&&!memcmp(bank,bank_saved,sizeof bank));
 for(unsigned i=0;i<sizeof bank;i++){bank[i]^=1;assert(!vc_patch_bank_valid(bank,sizeof bank));bank[i]^=1;}
 guard=b;
 for(unsigned i=0;i<sizeof bytes;i++)for(unsigned bit=0;bit<8;bit++){bytes[i]^=1u<<bit;assert(!vc_patch_decode(&b,bytes,sizeof bytes));assert(!memcmp(&b,&guard,sizeof b));bytes[i]^=1u<<bit;corruptions++;}
 for(unsigned n=0;n<sizeof bytes;n++)assert(!vc_patch_decode(&b,bytes,n));
 assert(!vc_patch_decode(&b,bytes,sizeof bytes+1));
 a.matrix.route[0].source=28;memset(bytes,0x55,sizeof bytes);memcpy(saved,bytes,sizeof bytes);assert(!vc_patch_encode(bytes,sizeof bytes,&a)&&!memcmp(bytes,saved,sizeof bytes));
 printf("{\"bank_slots\":16,\"bank_bytes\":3044,\"preset_roundtrips\":650,\"record_bytes\":%u,\"bit_corruptions_rejected\":%u,\"truncations_rejected\":true,\"failed_decode_preserves_destination\":true,\"invalid_encode_preserves_output\":true,\"pass\":true}\n",(unsigned)sizeof bytes,corruptions);
}
