#define VC_ACCESS_UI 1
#define APP_SPLASH
void app_splash(void);
#define VC_DUAL_RENDER 1
#define APP_OVERLOAD 1   /* shed a releasing voice after a block over 90% (app.c) */
#define VC_BAND_CACHE 1
#ifndef PLAT_HOST
#define VC_BAND_FM1_SINGLE_CORE 1
#endif
#include "../app/app.h"
#undef APP_CANVAS_PX
#define APP_CANVAS_PX (120u * 86u)
#undef APP_BLOCK
#define APP_BLOCK 512u
#undef APP_NAME
#define APP_NAME "CONTAGION"
#undef APP_TAGLINE
#define APP_TAGLINE "CONTAGION 8V"

#define APP_SYSEX
int app_sysex(const uint8_t*,uint32_t);
