#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 const unsigned notes[]={24,60,96},tunes[]={16,64,112},detunes[]={0,64,127},shapes[]={0,64,127};const float values[]={-1,0,1};
 for(unsigned mode=1;mode<=7;mode++)for(unsigned n=0;n<3;n++)for(unsigned tune=0;tune<3;tune++)for(unsigned det=0;det<3;det++)for(unsigned sh=0;sh<3;sh++)for(unsigned val=0;val<3;val++){
  VCSynth s;vc_init(&s,44100);s.param[VC_OSC3_MODE]=mode;s.param[VC_OSC3_TUNE]=tunes[tune];s.param[VC_OSC3_DETUNE]=detunes[det];s.param[VC_SHAPE2]=shapes[sh];s.param[VC_WAVE2]=3;s.param[VC_PW2]=80;s.param[VC_TUNE2]=70;s.param[VC_OSC2_ENV]=80;s.param[VC_LFO_OSC2]=80;s.param[VC_LFO_PW]=80;s.param[VC_LFO2_SHAPE_AMOUNT]=96;
  vc_note_on(&s,notes[n],100);VCVoice *v=&s.voice[0];v->filter_env.level=.5f;v->lfo_mod=values[val];v->lfo2_mod=values[val];vc_lfo2_osc_control(&s,v);vc_pitch_env_control(&s,v);vc_osc3_control(&s,v);
  emit(v->inc3);emit(v->osc3.wave_gain);emit(v->osc3.saw_gain);emit(v->osc3.pulse_gain);emit(v->osc3.duty);emit(v->osc3_gain);
  if(mode<4)assert(v->osc3.tables==v->osc[1].tables);else assert(v->osc3.tables==vc_wave_views[mode-4]);assert(v->osc3.table==v->osc3.tables[v->osc3.band]);
 }
 for(unsigned route=0;route<4;route++)for(unsigned mode=1;mode<=7;mode++){
  VCSynth s;vc_init(&s,44100);s.param[VC_ROUTING]=route;s.param[VC_OSC3_MODE]=mode;s.param[VC_SHAPE2]=32;s.param[VC_PW2]=80;s.param[VC_CUTOFF]=73;s.param[VC_CUTOFF2]=91;s.param[VC_RESONANCE]=55;s.param[VC_RESONANCE2]=70;vc_route_setup(&s);vc_note_on(&s,60,127);VCVoice *v=&s.voice[0];vc_filter_control(&s,v);
  v->inc[0]=1.f/128;v->inc[1]=1.f/64;v->inc3=1.f/256;v->control_remaining=10000;v->env.level=v->env.sustain=1;v->env.stage=3;v->filter_env.level=0;
  vc_osc_setup(&v->osc[0],0,64,.5f,v->inc[0]);vc_osc_setup(&v->osc[1],0,32,.5f*(1-80.f/127),v->inc[1]);v->osc3.band=vc_osc_band(v->inc3);v->osc3.table=v->osc3.tables[v->osc3.band];
  for(unsigned i=0;i<2048;i++){float l,r;vc_render_stereo(&s,&l,&r,1);emit(l);emit(r);}
 }
 /* Phase settings preserve the original third oscillator relationship. */
 for(unsigned phase=0;phase<=128;phase++){
  VCSynth s;vc_init(&s,44100);s.param[VC_OSC3_MODE]=1;s.param[VC_INIT_PHASE]=phase;vc_note_on(&s,60,100);VCVoice *v=&s.voice[0];assert(v->osc3.phase>=0&&v->osc3.phase<1);
  if(phase&&phase<128)assert(v->osc3.phase==1-phase/128.f);
  if(phase==128)assert(v->osc3.phase==0);
 }
}
