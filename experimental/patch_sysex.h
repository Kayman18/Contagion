#ifndef VC_PATCH_SYSEX_H
#define VC_PATCH_SYSEX_H
#include <stdint.h>
#define VC_PATCH_WIRE_MAX 226u
unsigned vc_patch_sysex_encode(uint8_t*,unsigned,unsigned,unsigned,const uint8_t*,unsigned);
int vc_patch_sysex_decode(const uint8_t*,unsigned,unsigned*,unsigned*,uint8_t*,unsigned*);
#endif
