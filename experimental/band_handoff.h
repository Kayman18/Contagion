#ifndef VC_EXPERIMENTAL_BAND_HANDOFF_H
#define VC_EXPERIMENTAL_BAND_HANDOFF_H
#include "band_cache.h"
#include "band_publication.h"
/* Single audio owner and single background owner. No concurrent reset/init.
 * Audio calls begin, captures corrections while old tables remain pinned, then
 * calls end before returning. boundary combines these for non-render clients.
 * Target atomic support/lock-freedom must be verified before firmware use. */
typedef struct {
 VCBandCache cache; /* background-owned, including all reference counts */
 VCBandWord cold[VC_BAND_CLIENTS];
 VCBandWord desired[VC_BAND_CLIENTS],published,acknowledged;
 int proposed_slot[VC_BAND_CLIENTS];
 const float *proposed_table[VC_BAND_CLIENTS];
 unsigned proposed_key[VC_BAND_CLIENTS];
 unsigned sequence,pending; /* background-owned */
 const float *audio_table[VC_BAND_CLIENTS];
 unsigned audio_key[VC_BAND_CLIENTS],audio_sequence; /* audio-owned */
} VCBandHandoff;
#define VC_BAND_OFF (~0u)
void vc_band_handoff_init(VCBandHandoff*,float tables[VC_BAND_SLOTS][2049],const double*,VCBandLoad,void*);
int vc_band_audio_request(VCBandHandoff*,unsigned client,unsigned wave,unsigned band);
void vc_band_audio_release(VCBandHandoff*,unsigned client);
int vc_band_audio_begin(VCBandHandoff*);
void vc_band_audio_end(VCBandHandoff*);
int vc_band_audio_boundary(VCBandHandoff*);
void vc_band_background(VCBandHandoff*,unsigned budget);
#endif
