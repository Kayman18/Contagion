#ifndef VC_VOICES_H
#define VC_VOICES_H
/* The firmware build generates voice_count.h. VC_VOICES can override it. */
#ifndef VC_VOICES
#include "voice_count.h"
#define VC_VOICES VC_VOICES_BUILD
#endif
#if VC_VOICES < 1 || VC_VOICES > 16
#error VC_VOICES must be 1..16 (cache client masks hold 3 clients per voice in 64 bits)
#endif
#endif
