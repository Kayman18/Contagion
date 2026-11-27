#ifndef VC_PRESETS_H
#define VC_PRESETS_H

#include "virooos.h"
#include "matrix.h"

typedef struct {
    uint8_t params[VC_PARAMS];
    VCMatrix matrix;
} VCFactoryPreset;

int vc_preset_decode(unsigned index, VCFactoryPreset *preset);
unsigned vc_preset_count(void);
unsigned vc_preset_bank_count(void);
const char *vc_preset_bank_label(unsigned bank);
const char *vc_preset_name(unsigned index);
unsigned vc_preset_bank(unsigned index);
unsigned vc_preset_program(unsigned index);
unsigned vc_preset_partial(unsigned index);
unsigned vc_preset_wave_cache(unsigned index);
int vc_preset_find(unsigned bank, unsigned original_program);
unsigned vc_preset_bank_first(unsigned bank);

#endif
