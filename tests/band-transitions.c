#include "../experimental/band_voice.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static float reference[64][14][2049];
static VCSynth synth;
static VCBandHandoff handoff;
static VCBandVoiceBinding binding;
static float lookup(const float *t,float p){float a=p*2048;unsigned i=(unsigned)a;return t[i]+(t[i+1]-t[i])*(a-i);}
int main(void){
 FILE *f=fopen("data/source-waves.f32","rb");assert(f&&fread(reference,sizeof reference,1,f)==1);fclose(f);
 vc_init(&synth,44100);vc_band_voice_attach(&binding,&handoff,&synth);
 VCVoice *v=&synth.voice[0];VCOsc *o=&v->osc[0];v->env.stage=1;v->age=1;o->wave_gain=1;o->band=0;o->table=vc_band_silence;
 for(unsigned c=0;c<12;c++){vc_band_word_init(&handoff.cold[c],0);vc_band_word_init(&handoff.desired[c],VC_BAND_OFF);handoff.audio_key[c]=VC_BAND_OFF;}
 float peak_error=0;
 for(unsigned n=0;n<4096;n++){
  unsigned w=(n*17)%64;synth.param[VC_WAVE1]=w;
  float before=lookup(o->table,o->phase)+o->table_correction*(o->table_ramp/128.f);
  handoff.audio_table[0]=reference[w][0];handoff.audio_key[0]=w*14;
  vc_band_voice_control(&synth,0);
  float after=vc_osc_next(o,0,-1),error=fabsf(after-before);if(error>peak_error)peak_error=error;assert(error<.000002f);
  unsigned frames=n%173;
  for(unsigned j=0;j<frames;j++)assert(isfinite(vc_osc_next(o,.0037f,-1)));
 }
 while(o->table_ramp)vc_osc_next(o,0,-1);
 assert(vc_osc_next(o,0,-1)==lookup(o->table,o->phase));
 /* A pitch change can retain a less harmonic table, never a richer one. */
 o->band=4;handoff.audio_key[0]=synth.param[VC_WAVE1]*14+13;handoff.audio_table[0]=reference[synth.param[VC_WAVE1]][13];vc_band_voice_control(&synth,0);assert(o->table==handoff.audio_table[0]);
 handoff.audio_key[0]=synth.param[VC_WAVE1]*14+2;vc_band_voice_control(&synth,0);assert(o->table==vc_band_silence);
 printf("{\"replacement_tests\":4096,\"max_step_error\":%.9g,\"finite_ramp_samples\":128,\"settled_exact\":true,\"safe_pitch_fallback\":true,\"pass\":true}\n",peak_error);
}
