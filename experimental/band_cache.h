#ifndef VC_EXPERIMENTAL_BAND_CACHE_H
#define VC_EXPERIMENTAL_BAND_CACHE_H
#include <stdint.h>
#include "band_rebuild.h"
#define VC_BAND_SLOTS 32
#include "../engine/voices.h"
/* Three oscillator clients per voice. */
#define VC_BAND_CLIENTS (3*VC_VOICES)
/* Serialized prototype. ALL calls require one owner or external exclusion.
 * This is not a lock-free audio/main-loop handoff. Render may retain a pointer
 * until commit/release for that client, which must occur at a block boundary. */
typedef int (*VCBandLoad)(void *context,unsigned wave,double coefficients[256],float *peak);
typedef struct {int wave,band;unsigned state,refs;} VCBandSlot;
typedef struct {int wave,band,slot;} VCBandClient;
typedef struct {
 VCBandSlot slots[VC_BAND_SLOTS];
 VCBandClient clients[VC_BAND_CLIENTS];
 float (*tables)[2049];const double *sines;
 VCBandLoad load;void *context;
 double coefficients[256];VCBandRebuild builder;
 uint64_t cold_clients; /* client bitmasks: up to 64 clients */
 unsigned cold_fallback;uint64_t building_clients,last_completed_clients;
 int reserved[3],last_completed;
 int building;unsigned next_client,next_slot,completed,cancelled,failures;
} VCBandCache;
void vc_band_cache_init(VCBandCache*,float tables[VC_BAND_SLOTS][2049],const double*,VCBandLoad,void*);
int vc_band_cache_request(VCBandCache*,unsigned client,unsigned wave,unsigned band);
int vc_band_cache_commit(VCBandCache*,unsigned client);
void vc_band_cache_release(VCBandCache*,unsigned client);
const float *vc_band_cache_current(const VCBandCache*,unsigned client);
/* Budget bounds harmonic steps only. Starting a job calls load once; its
 * coefficient decoding cost is separate and must be bounded by integration. */
int vc_band_cache_service(VCBandCache*,unsigned budget);
#endif
