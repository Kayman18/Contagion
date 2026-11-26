#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 const unsigned notes[]={24,60,96},transposes[]={0,52,64,76,127},ranges[]={0,52,62,64,66,76,127},bends[]={0,2048,4096,6144,8192,10240,12288,14336,16383};
 for(unsigned note=0;note<3;note++)for(unsigned trans=0;trans<5;trans++)for(unsigned range=0;range<7;range++)for(unsigned scale=0;scale<2;scale++)for(unsigned bend=0;bend<9;bend++)for(unsigned mode=1;mode<=4;mode++){
  VCSynth s;vc_init(&s,44100);s.param[VC_TRANSPOSE]=transposes[trans];s.param[VC_BEND_UP]=ranges[range];s.param[VC_BEND_DOWN]=127-ranges[range];s.param[VC_BEND_SCALE]=scale;s.param[VC_OSC3_MODE]=mode;
  s.param[VC_KEYFOLLOW1]=80;s.param[VC_KEYFOLLOW2]=112;s.param[VC_TUNE1]=70;s.param[VC_TUNE2]=52;s.param[VC_OSC3_TUNE]=76;s.param[VC_F_KEYFOLLOW1]=96;s.param[VC_F_KEYFOLLOW2]=32;s.param[VC_CUTOFF]=73;s.param[VC_CUTOFF2]=91;s.param[VC_SAT_CURVE]=10;s.param[VC_SAT_DRIVE]=96;
  vc_bend(&s,bends[bend]);vc_note_on(&s,notes[note],100);VCVoice *v=&s.voice[0];vc_filter_control(&s,v);
  emit(v->inc[0]);emit(v->inc[1]);emit(v->inc3);emit(v->filter[0].cutoff);emit(v->filter[1].cutoff);emit(v->saturation.period);
 }
 /* Held notes update without retriggering envelopes; LFO keytrack uses MIDI note. */
 VCSynth s;vc_init(&s,44100);vc_note_on(&s,60,100);VCVoice *v=&s.voice[0];float rate=v->lfo.rate;v->env.level=.75f;v->env.stage=3;float inc=v->inc[0];
 vc_param(&s,VC_TRANSPOSE,76);assert(fabsf(v->inc[0]-2*inc)<1e-6f&&v->env.level==.75f&&v->env.stage==3&&v->lfo.rate==rate);
 vc_param(&s,VC_BEND_UP,76);vc_bend(&s,12288);assert(fabsf(s.pitch_offset-18)<1e-6f);
 vc_param(&s,VC_BEND_SCALE,1);assert(fabsf(s.pitch_offset-15)<1e-6f);
 vc_param(&s,VC_BEND_DOWN,40);vc_bend(&s,4096);assert(fabsf(s.pitch_offset-6)<1e-6f);
}
