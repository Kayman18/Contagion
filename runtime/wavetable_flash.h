#pragma once
#include <stdint.h>
#include "wavetable_flash_layout.h"

void vc_wave_flash_init(const uint8_t *image);
int vc_wave_flash_copy(unsigned wave, unsigned band, float table[2049]);
unsigned vc_wave_flash_hits(void);
