#ifndef VC_EXPERIMENTAL_BAND_VOICE_H
#define VC_EXPERIMENTAL_BAND_VOICE_H
#include "../engine/virooos.h"
#include "band_handoff.h"
typedef struct {VCBandHandoff *handoff;VCSynth *synth;uint32_t age[VC_BAND_CLIENTS];unsigned ready[VC_BAND_CLIENTS];
#ifdef VC_DUAL_RENDER
 unsigned desired[VC_BAND_CLIENTS],cold[VC_BAND_CLIENTS];
#endif
} VCBandVoiceBinding;
extern const float vc_band_silence[2049];
/* Single attached synth. Attach/detach only while rendering is stopped. */
void vc_band_voice_attach(VCBandVoiceBinding*,VCBandHandoff*,VCSynth*);
void vc_band_voice_detach(void);
void vc_band_voice_boundary(VCSynth*);
void vc_band_voice_publish(VCSynth*);
void vc_band_wave_control(const VCMatrix*,const float*,const uint8_t*,VCOsc*);
void vc_band_voice_control(VCSynth*,unsigned voice);
#endif
