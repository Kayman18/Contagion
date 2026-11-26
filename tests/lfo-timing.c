#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 const unsigned rates[]={0,64,127},tempos[]={0,57,127};const float deviations[]={.98f,1,1.02f};
 for(unsigned note=0;note<128;note++)for(unsigned track=0;track<128;track++)for(unsigned r=0;r<3;r++){
  emit(vc_lfo_rate(vc_lfoRate[rates[r]],note,track,1.02f,0,57));
 }
 for(unsigned clock=1;clock<=21;clock++)for(unsigned tempo=0;tempo<128;tempo++)for(unsigned d=0;d<3;d++)emit(vc_lfo_rate(999,127,127,deviations[d],clock,tempo));
 const unsigned clocks[]={0,5,17},notes[]={36,84};
 for(unsigned c=0;c<3;c++)for(unsigned n=0;n<2;n++)for(unsigned mode=1;mode<=2;mode++){
  VCSynth s;vc_init(&s,44100);s.param[VC_LFO_CLOCK]=clocks[c];s.param[VC_TEMPO]=tempos[c];s.param[VC_LFO_KEYTRACK]=96;s.param[VC_LFO_RATE]=96;s.param[VC_LFO_MODE]=mode;
  vc_note_on(&s,notes[n],100);VCVoice *v=&s.voice[0];v->lfo_deviation=1;vc_update_voice(&s,v);
  for(unsigned i=0;i<8000;i++){
   if(i==4000){float phase=v->lfo.phase;vc_param(&s,VC_TEMPO,100);vc_param(&s,VC_LFO_KEYTRACK,32);assert(v->lfo.phase==phase);}
   float l,r;vc_render_stereo(&s,&l,&r,1);emit(v->lfo.rate);emit(v->lfo.phase);emit(v->lfo.value);
  }
 }
}
