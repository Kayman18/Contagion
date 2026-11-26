#include "virooos.h"
#include "presets.h"
#include "presets.inc"
#ifdef VC_BAND_CACHE
#include "../experimental/preset_status.inc"
#endif
unsigned vc_preset_count(void){return VC_PRESET_COUNT;}
unsigned vc_preset_bank_count(void){return VC_PRESET_BANK_COUNT;}
char vc_preset_bank_letter(unsigned bank){return (char)(VC_PRESET_FIRST_BANK+bank);}
const uint8_t *vc_preset_params(unsigned i){return i<VC_PRESET_COUNT?preset_params[i]:0;}
const char *vc_preset_name(unsigned i){return i<VC_PRESET_COUNT?preset_names[i]:"INIT";}
unsigned vc_preset_bank(unsigned i){return i<VC_PRESET_COUNT?preset_banks[i]:0;}
unsigned vc_preset_program(unsigned i){return i<VC_PRESET_COUNT?preset_programs[i]:0;}
unsigned vc_preset_partial(unsigned i){
#ifdef VC_BAND_CACHE
 return i<VC_PRESET_COUNT?(preset_status_flags[i]&31)!=0:0;
#else
 return i<VC_PRESET_COUNT?preset_partial[i]:0;
#endif
}
unsigned vc_preset_wave_cache(unsigned i){
#ifdef VC_BAND_CACHE
 return i<VC_PRESET_COUNT?(preset_status_flags[i]&32)!=0:0;
#else
 (void)i;return 0;
#endif
}
int vc_preset_find(unsigned bank,unsigned program){for(unsigned i=0;i<VC_PRESET_COUNT;i++)if(preset_banks[i]==bank&&preset_programs[i]==program)return (int)i;return -1;}
unsigned vc_preset_bank_first(unsigned bank){for(unsigned i=0;i<VC_PRESET_COUNT;i++)if(preset_banks[i]==bank)return i;return 0;}
