#include "../experimental/band_handoff.h"
char vc_band_target_handoff_bytes[sizeof(VCBandHandoff)];
char vc_band_target_cache_bytes[sizeof(VCBandCache)];
char vc_band_target_builder_bytes[sizeof(VCBandRebuild)];
char vc_band_target_mailbox_alignment[_Alignof(VCBandWord)];
