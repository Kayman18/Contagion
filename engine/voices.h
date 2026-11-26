#ifndef VC_VOICES_H
#define VC_VOICES_H
/* Physical voices. Precedence: -DVC_VOICES=n (tests), an app header that
 * defines it (the single-core TEST74 app keeps 4), then voice_count.h,
 * which build-band-firmware.sh writes from VOICES (default 12). */
#ifndef VC_VOICES
#include "voice_count.h"
#define VC_VOICES VC_VOICES_BUILD
#endif
#if VC_VOICES < 1 || VC_VOICES > 16
#error VC_VOICES must be 1..16 (cache client masks hold 3 clients per voice in 64 bits)
#endif
#endif
