#ifndef VC_MATRIX_H
#define VC_MATRIX_H
#include <stdint.h>
/* Source indices and destination indices retain the original patch format.
 * TEST59 consumes forty-two filter/pan/FM/shape/PWM/pitch/amp destinations in the voice engine. */
#define VC_MATRIX_SOURCES 28
#define VC_MATRIX_DESTINATIONS 123
#define VC_MATRIX_ROUTES 11
typedef struct { uint8_t source, destination, amount; } VCMatrixRoute;
typedef struct { VCMatrixRoute route[VC_MATRIX_ROUTES]; } VCMatrix;
/* base/effective contain values in destination-ID order, including route
 * amount destinations. They must be separate arrays. No output clamping:
 * each consuming DSP parameter applies its original domain conversion. */
void vc_matrix_apply(const VCMatrix *, const float sources[VC_MATRIX_SOURCES],
                     const float base[VC_MATRIX_DESTINATIONS],
                     float effective[VC_MATRIX_DESTINATIONS]);
const VCMatrix *vc_matrix_factory(unsigned preset);
int vc_matrix_cc_source(unsigned cc);
/* Output order: destinations 24..31, then 76..79 (filter velocity amounts), then 48,49,54,55, then 32 (filter balance), then 3,56,60,81 (pan), then 16,18,53,75 (FM), then 6,11,52,72,73 (shape), then 7,12,47,74 (PWM), then 9,14,17,45,46 (pitch), then 1,2,80 (amplitude).
 * All route-amount dependencies are evaluated before these sound targets. */
#define VC_MATRIX_FILTERS 73
unsigned vc_matrix_rounded(const VCMatrix *,const float sources[28],unsigned destination,float base);
void vc_matrix_filters(const VCMatrix *,const float sources[28],float delta[VC_MATRIX_FILTERS]);
int vc_matrix_has_filters(const VCMatrix *);
#endif
