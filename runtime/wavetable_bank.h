#ifndef VC_EXPERIMENTAL_BAND_BANK_H
#define VC_EXPERIMENTAL_BAND_BANK_H
#include <stdint.h>
typedef struct {const uint8_t *data;unsigned bytes;} VCBandBank;
/* Input must remain immutable and accessible until cache shutdown. Init runs
 * outside audio; validates the bank and its binding to normalization peaks. */
int vc_band_bank_init(VCBandBank*,const uint8_t*,unsigned);
/* VCBandLoad-compatible background callback. Decodes one bounded 2048-byte
 * coefficient block and checks its CRC. Never called by audio. */
int vc_band_bank_load(void*,unsigned wave,double coefficients[256],float *peak);
int vc_lz4_unpack(const uint8_t *src,unsigned bytes,uint8_t *dst,unsigned size);
#endif
