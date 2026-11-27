#include "presets.h"
#include "../runtime/wavetable_bank.h"
#include <string.h>
#include "packed_presets.inc"

#define PRESET_BLOCK 32u
_Static_assert(sizeof(VCFactoryPreset) == 159, "Packed preset layout changed");
#ifdef PLAT_HOST
static uint8_t cache[PRESET_BLOCK * sizeof(VCFactoryPreset)];
#else
static uint8_t cache[PRESET_BLOCK * sizeof(VCFactoryPreset)] __attribute__((section(".pool"), aligned(8)));
#endif
static unsigned cached_block = ~0u;

int vc_preset_decode(unsigned index, VCFactoryPreset *preset)
{
    if (index >= VC_PRESET_COUNT) return index == VC_PRESET_COUNT;
    unsigned block = index / PRESET_BLOCK, first = block * PRESET_BLOCK;
    unsigned count = VC_PRESET_COUNT - first;
    if (count > PRESET_BLOCK) count = PRESET_BLOCK;
    if (block != cached_block) {
        unsigned offset = packed_offsets[block], end = packed_offsets[block + 1];
        cached_block = ~0u;
        if (!vc_lz4_unpack(packed_presets + offset, end - offset, cache, count * sizeof(*preset))) return 0;
        cached_block = block;
    }

    uint8_t bytes[sizeof(*preset)];
    const uint8_t *field = cache + index - first;
    for (unsigned i = 0; i < sizeof(bytes); i++) bytes[i] = field[i * count];
    memcpy(preset, bytes, sizeof(bytes));
    return 1;
}

unsigned vc_preset_count(void) { return VC_PRESET_COUNT; }
unsigned vc_preset_bank_count(void) { return VC_PRESET_BANK_COUNT; }
const char *vc_preset_bank_label(unsigned bank) { return bank < VC_PRESET_BANK_COUNT ? bank_labels[bank] : ""; }
const char *vc_preset_name(unsigned i) { return i < VC_PRESET_COUNT ? preset_names[i] : "INIT"; }
unsigned vc_preset_bank(unsigned i) { return i < VC_PRESET_COUNT ? preset_banks[i] : 0; }
unsigned vc_preset_program(unsigned i) { return i < VC_PRESET_COUNT ? preset_programs[i] : 0; }
unsigned vc_preset_partial(unsigned i) { return i < VC_PRESET_COUNT && (preset_status[i] & 31u); }
unsigned vc_preset_wave_cache(unsigned i) { return i < VC_PRESET_COUNT && (preset_status[i] & 32u); }

int vc_preset_find(unsigned bank, unsigned program)
{
    if (bank < VC_PRESET_BANK_COUNT)
        for (unsigned i = bank_first[bank]; i < bank_first[bank + 1]; i++)
            if (preset_programs[i] == program) return (int)i;
    return -1;
}

unsigned vc_preset_bank_first(unsigned bank)
{
    return bank < VC_PRESET_BANK_COUNT ? bank_first[bank] : 0;
}
