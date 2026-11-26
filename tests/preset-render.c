#include "virooos.h"
#include "presets.h"
#include "wave_cache.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
static float extra[22*2049];
int main(void){FILE *f=fopen("data/VIROOOS-LFO-EXTRA.bin","rb");assert(f&&fread(extra,sizeof(extra),1,f)==1);fclose(f);vc_lfo_set_extra_tables(extra);assert(vc_wave_init(vc_wave_seed(),vc_wave_seed_size()));
 unsigned nonzero=0;float maximum=0;
 for(unsigned i=0;i<vc_preset_count();i++){
  VCSynth s;vc_init(&s,44100);vc_apply_patch(&s,vc_preset_params(i));
#ifdef VC_MATRIX
 vc_set_matrix(&s,vc_matrix_factory(i));vc_cc(&s,1,127);vc_pressure(&s,99);
#endif
unsigned w[3]={s.param[VC_WAVE1],s.param[VC_WAVE2],s.param[VC_OSC3_MODE]>=4?s.param[VC_OSC3_MODE]-4:s.param[VC_WAVE2]};
  for(unsigned j=0;j<3;j++){if(vc_wave_cached(j)==(int)w[j])continue;assert(vc_wave_begin(j,w[j]));int result;do{result=vc_wave_service(65536);}while(result==0);assert(result==1);}
  for(unsigned j=0;j<4;j++)vc_note_on(&s,48+4*j,100);float peak=0;
  for(unsigned block=0;block<16;block++){float l[256],r[256];vc_render_stereo(&s,l,r,256);for(unsigned j=0;j<256;j++){assert(isfinite(l[j])&&isfinite(r[j]));float p=fmaxf(fabsf(l[j]),fabsf(r[j]));if(p>peak)peak=p;}}
  nonzero+=peak>0.000001f;if(peak>maximum)maximum=peak;
 }
 printf("{\"presets\":650,\"stereo_frames_per_preset\":4096,\"all_samples_finite\":true,\"nonzero_output_in_first_93ms\":%u,\"peak\":%.9g,\"pass\":true}\n",nonzero,maximum);
}
