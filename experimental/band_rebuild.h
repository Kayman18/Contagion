#ifndef VC_EXPERIMENTAL_BAND_REBUILD_H
#define VC_EXPERIMENTAL_BAND_REBUILD_H
/* Background-only reconstruction. Caller owns all buffers and
 * must keep output unpublished until service returns 1. No allocation. */
typedef struct {
 const double *coefficients,*sines;
 float *output;
 double accumulator[32],peak;
 unsigned sample,harmonic,harmonics,active,tile_index;
 unsigned long operations;
} VCBandRebuild;
int vc_band_begin(VCBandRebuild*,const double*,const double*,float peak,unsigned band,float output[2049]);
int vc_band_service(VCBandRebuild*,unsigned budget);
#endif
