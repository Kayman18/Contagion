#include "wavetable_flash.h"
#include <string.h>

#define FLASH_TABLES 34u
#define TABLE_BYTES (2049u * 4u)
#define HEADER_BYTES 20u
#define ENTRY_BYTES 8u
#define DATA_START (HEADER_BYTES + FLASH_TABLES * ENTRY_BYTES)

static const uint8_t *bank;
static uint8_t verified[FLASH_TABLES];
static unsigned hits;

static uint32_t read32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
           (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static uint32_t checksum(const uint8_t *bytes, unsigned count)
{
    static const uint32_t nibble[] = {
        0,0x1db71064,0x3b6e20c8,0x26d930ac,0x76dc4190,0x6b6b51f4,0x4db26158,0x5005713c,
        0xedb88320,0xf00f9344,0xd6d6a3e8,0xcb61b38c,0x9b64c2b0,0x86d3d2d4,0xa00ae278,0xbdbdf21c
    };
    uint32_t crc = ~0u;
    while (count--) {
        crc ^= *bytes++;
        crc = (crc >> 4) ^ nibble[crc & 15u];
        crc = (crc >> 4) ^ nibble[crc & 15u];
    }
    return ~crc;
}

void vc_wave_flash_init(const uint8_t *image)
{
    bank = 0;
    hits = 0;
    memset(verified, 0, sizeof(verified));
    if (!image || memcmp(image, "CTWAVE01", 8) ||
        read32(image + 8) != FLASH_TABLES || read32(image + 12) != TABLE_BYTES ||
        checksum(image + HEADER_BYTES, FLASH_TABLES * ENTRY_BYTES) != read32(image + 16))
        return;
    bank = image;
}

unsigned vc_wave_flash_hits(void) { return hits; }

int vc_wave_flash_copy(unsigned wave, unsigned band, float table[2049])
{
    if (!bank) return 0;
    for (unsigned i = 0; i < FLASH_TABLES; i++) {
        const uint8_t *entry = bank + HEADER_BYTES + i * ENTRY_BYTES;
        if (entry[0] != wave || entry[1] != band || verified[i] == 2) continue;
        const uint8_t *source = bank + DATA_START + i * TABLE_BYTES;
        memcpy(table, source, TABLE_BYTES);
        if (!verified[i] && checksum((const uint8_t *)table, TABLE_BYTES) != read32(entry + 4)) {
            verified[i] = 2;
            return 0;
        }
        verified[i] = 1;
        hits++;
        return 1;
    }
    return 0;
}
