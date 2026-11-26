#ifndef VC_DISTORTION_H
#define VC_DISTORTION_H
#include <stdint.h>
/* Stereo post-mix effect, independent of per-voice filter saturation. */
#ifdef PLAT_HOST
#define VC_DIST_HOT
#else
#define VC_DIST_HOT __attribute__((section(".voice_hot")))
#endif
typedef struct {
 unsigned mode,level;
 float drive,gain,quantum,coef_a,coef_b;
 float previous[2],filtered[2],held[2];
 uint32_t counter[2],step;
} VCDistortion;
void vc_distortion_reset(VCDistortion *);
void vc_distortion_set(VCDistortion *,unsigned mode,unsigned level);
VC_DIST_HOT float vc_distortion_next(VCDistortion *,float sample,unsigned channel);
#endif
