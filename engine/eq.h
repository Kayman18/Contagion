#ifndef VC_EQ_H
#define VC_EQ_H
#include <stdint.h>
/* Original low shelf, parametric mid and high shelf; plus original Analog Boost. */
#ifdef PLAT_HOST
#define VC_EQ_HOT
#else
#define VC_EQ_HOT __attribute__((section(".voice_hot")))
#endif
/* split[] caches Dekker splits (high, low) that vc_eq_next would recompute:
 * the five mid coefficients, and per channel the splits of x1, x2, y1, y2. */
typedef struct {float mid[5],low[3],high[3],normalization;float history[2][8];float mid_error[2][2];
 float mid_split[5][2];float history_split[2][4][2];} VCEQ;
typedef struct {float coef[3],history[2][2];unsigned enabled;} VCBoost;
void vc_boost_set(VCBoost *,unsigned,unsigned);
VC_EQ_HOT float vc_boost_next(VCBoost *,float,unsigned);
void vc_eq_reset(VCEQ *);
void vc_eq_set(VCEQ *,unsigned low_gain,unsigned low_frequency,unsigned mid_gain,unsigned mid_frequency,unsigned mid_q,unsigned high_gain,unsigned high_frequency);
VC_EQ_HOT float vc_eq_next(VCEQ *,float,unsigned);
/* Copy one channel's running state (used by the two-core post stage). */
void vc_eq_commit_channel(VCEQ *to,const VCEQ *from,unsigned channel);
#endif
