#include "../experimental/band_voice.h"
#include "../experimental/band_bank.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
static VCBandHandoff h;static VCBandVoiceBinding binding;static float tables[VC_BAND_SLOTS][2049],reference[64][14][2049];static double sines[2048];static unsigned char bankbytes[90426];static VCSynth synth;
static void read_file(const char *p,void *data,size_t n){FILE *f=fopen(p,"rb");assert(f&&fread(data,n,1,f)==1);fclose(f);}
static void render(void){float l[32],r[32];vc_render_stereo(&synth,l,r,32);for(unsigned i=0;i<32;i++)assert(isfinite(l[i])&&isfinite(r[i]));}
static void settle(void){for(unsigned n=0;n<2000;n++){vc_band_background(&h,8192);render();unsigned ready=1;for(unsigned i=0;i<12;i++){unsigned wanted=vc_band_word_load(&h.desired[i]);ready&=wanted==h.audio_key[i];}if(ready)return;}assert(!"voice cache did not settle");}
int main(void){
 read_file("data/VIROOOS-WAV.bin",bankbytes,sizeof(bankbytes));read_file("data/source-waves.f32",reference,sizeof(reference));read_file("data/wave-sines.f64",sines,sizeof(sines));VCBandBank bank;assert(vc_band_bank_init(&bank,bankbytes,sizeof(bankbytes)));vc_band_handoff_init(&h,tables,sines,vc_band_bank_load,&bank);vc_init(&synth,44100);vc_band_voice_attach(&binding,&h,&synth);
 vc_param(&synth,VC_SHAPE1,0);vc_param(&synth,VC_SHAPE2,0);vc_param(&synth,VC_OSC3_MODE,11);vc_param(&synth,VC_WAVE1,5);vc_param(&synth,VC_WAVE2,9);for(unsigned i=0;i<4;i++)vc_note_on(&synth,36+i*12,100);render();assert(synth.voice[0].osc[0].table==vc_band_silence);settle();
 for(unsigned i=0;i<12;i++){assert(h.audio_table[i]);unsigned key=h.audio_key[i];assert(!memcmp(h.audio_table[i],reference[key/14][key%14],8196));}
 /* Compare settled output to the exact full-table reference using identical state. */
 VCSynth baseline=synth;for(unsigned v=0;v<4;v++)for(unsigned o=0;o<3;o++){VCOsc *osc=o<2?&baseline.voice[v].osc[o]:&baseline.voice[v].osc3;osc->table=reference[osc->wave][osc->band];}
 float l[256],r[256],ll[256],rr[256];vc_render_stereo(&synth,l,r,256);vc_render_stereo(&baseline,ll,rr,256);assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));
 /* Wave-only matrices must remain active; held voices must not retrigger. */
 VCMatrix matrix={0};for(unsigned k=0;k<11;k++)matrix.route[k].amount=64;
 matrix.route[0]=(VCMatrixRoute){3,8,127};matrix.route[1]=(VCMatrixRoute){4,13,0};
 vc_param(&synth,VC_OSC3_MODE,1);vc_set_matrix(&synth,&matrix);assert(synth.matrix==&matrix);
 unsigned age=synth.voice[0].age;
 for(unsigned step=0;step<5;step++){
  synth.controllers[3]=step*.25f;synth.controllers[4]=step*.25f;
  render();settle();unsigned w1=(unsigned)fmin(63,floor(5+126*(double)synth.controllers[3]+.5));
  unsigned w2=(unsigned)fmax(0,floor(9-128*(double)synth.controllers[4]+.5));
  for(unsigned v=0;v<4;v++){
   assert(synth.voice[v].osc[0].wave==(int)w1&&synth.voice[v].osc[1].wave==(int)w2);
   assert(h.audio_key[v*3]/14==w1&&h.audio_key[v*3+1]/14==w2&&h.audio_key[v*3+2]/14==w2);
  }assert(vc_active(&synth)==4&&synth.voice[0].age==age);
 }
 vc_set_matrix(&synth,0);render();settle();
 assert(synth.voice[0].osc[0].wave==5&&synth.voice[0].osc[1].wave==9);
 /* Same-band wave miss retains old band; after settling use new exact wave. */
 const float *old=synth.voice[0].osc[0].table;vc_param(&synth,VC_WAVE1,17);render();assert(synth.voice[0].osc[0].table==old);settle();assert(h.audio_key[0]/14==17);
 /* Force a frequency-band change: no unsuitable old harmonic band is read. */
 vc_param(&synth,VC_TUNE1,112);render();assert(synth.voice[0].osc[0].table==vc_band_silence);settle();
 vc_param(&synth,VC_OSC3_MODE,1);render();settle();for(unsigned i=0;i<4;i++)assert(h.audio_key[i*3+2]/14==synth.param[VC_WAVE2]);
 vc_param(&synth,VC_WAVE1,32);vc_note_on(&synth,100,100);render();unsigned stolen=4;for(unsigned i=0;i<4;i++)if(synth.voice[i].note==100)stolen=i;assert(stolen<4&&synth.voice[stolen].osc[0].table==vc_band_silence);settle();
 vc_param(&synth,VC_RELEASE,0);for(unsigned i=0;i<4;i++)vc_note_off(&synth,synth.voice[i].note);for(unsigned n=0;n<2000&&vc_active(&synth);n++)render();assert(!vc_active(&synth));settle();vc_band_background(&h,0);for(unsigned i=0;i<VC_BAND_SLOTS;i++)assert(!h.cache.slots[i].refs);
 vc_note_on(&synth,60,100);render();settle();vc_panic(&synth);render();settle();vc_band_background(&h,0);for(unsigned i=0;i<VC_BAND_SLOTS;i++)assert(!h.cache.slots[i].refs);
 vc_band_voice_detach();printf("{\"voices\":4,\"oscillator_clients\":12,\"settled_audio_byte_identical\":true,\"new_note_waits_for_correct_table\":true,\"same_band_miss_retains_old_wave\":true,\"wrong_pitch_band_not_used\":true,\"osc3_slave_sharing\":true,\"panic_releases_pins\":true,\"pass\":true}\n");
}
