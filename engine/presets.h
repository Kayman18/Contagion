#ifndef VC_PRESETS_H
#define VC_PRESETS_H
#include <stdint.h>
unsigned vc_preset_count(void);
unsigned vc_preset_bank_count(void);
char vc_preset_bank_letter(unsigned bank);
const uint8_t *vc_preset_params(unsigned);
const char *vc_preset_name(unsigned);
unsigned vc_preset_bank(unsigned);
unsigned vc_preset_program(unsigned);
unsigned vc_preset_partial(unsigned);
unsigned vc_preset_wave_cache(unsigned);
int vc_preset_find(unsigned bank,unsigned original_program);
unsigned vc_preset_bank_first(unsigned bank);
#endif
