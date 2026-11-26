#ifndef VC_WAVE_CACHE_H
#define VC_WAVE_CACHE_H
#include <stdint.h>
#define VC_WAVE_BANK_FLASH 0x97000u
const uint8_t *vc_wave_seed(void);
unsigned vc_wave_seed_size(void);
int vc_wave_bank_valid(const uint8_t *,unsigned);
int vc_wave_init(const uint8_t *,unsigned);
int vc_wave_begin(unsigned slot,unsigned wave);
int vc_wave_service(unsigned budget); /* 0 building, 1 finished, -1 invalid */
int vc_wave_cached(unsigned slot);
const float *const *vc_wave_tables(unsigned wave);
const float *vc_wave_table(unsigned wave,unsigned band);
unsigned vc_wave_progress(void);
uint32_t vc_wave_bytes_crc(const void *,unsigned);
#endif
