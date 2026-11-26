#pragma once
#include <stdint.h>
#include "plat.h"
#define VC_DYNAMIC_WAVES 1
#define VC_POST_FX 1
#define VC_MATRIX 1
/* The single-core TEST74 app keeps four voices (fm1.toml passes the same to
 * the engine units); the dual-core band app uses the 12-voice default. */
#ifndef VC_DUAL_RENDER
#define VC_VOICES 4
#endif
#define APP_NAME "VIROOOS"
#define APP_TAGLINE "TEST74"
#define APP_SPLASH_RGB RGB(210,35,40)
#define APP_PALETTE 3u
#define APP_BLOCK 256u
#define APP_CANVAS_PX 8192u
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
