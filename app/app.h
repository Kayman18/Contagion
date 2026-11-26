#pragma once
#include <stdint.h>
#include "plat.h"
#define VC_DYNAMIC_WAVES 1
#define VC_POST_FX 1
#define VC_MATRIX 1
#define VC_ACCESS_UI 1
#define VC_DUAL_RENDER 1
#define VC_BAND_CACHE 1
#define APP_OVERLOAD 1
#ifndef PLAT_HOST
#define VC_BAND_FM1_SINGLE_CORE 1
#endif
#define APP_NAME "CONTAGION"
#define APP_TAGLINE "CONTAGION 8V"
#define APP_SPLASH
void app_splash(void);
#define APP_SPLASH_RGB RGB(210,35,40)
#define APP_PALETTE 3u
#define APP_BLOCK 512u
#define APP_CANVAS_PX (120u * 86u)
#define APP_SYSEX
int app_sysex(const uint8_t*,uint32_t);
#define APP_TOUCH
#define APP_HOST_EXPECT
enum {OBJ_SETTINGS,OBJ_PANEL,OBJ_BLE,OBJ_NOBJ};
#define APP_STORE_MAP {0xFC000u,0xDC000u,0xDE000u}
void app_init(void);
void app_render(int32_t*,uint32_t,uint32_t);
void app_panic(void);
void app_ui_init(void);
void app_frame(void);
void app_input_only(void);
void app_say(const char*,const char*);
int app_expect(const char*,int*);
