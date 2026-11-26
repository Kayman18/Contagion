/* Include the engine to exercise its private control tick directly. */
#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 const float values[]={-1,-.25f,0,.5f,1,2};
 for(unsigned route=0;route<4;route++)for(unsigned mode=0;mode<4;mode++)for(unsigned amount=0;amount<128;amount++)for(unsigned j=0;j<6;j++){
  VCSynth s;vc_init(&s,44100);s.param[VC_ROUTING]=route;s.param[VC_FILTER_MODE]=mode;s.param[VC_FILTER_MODE2]=(mode+1)%4;
  s.param[VC_CUTOFF]=73;s.param[VC_CUTOFF2]=91;s.param[VC_RESONANCE]=55;s.param[VC_RESONANCE2]=70;
  s.param[VC_LFO_RESONANCE]=amount;s.param[VC_LFO_FILTER_GAIN]=amount;s.param[VC_LFO_MODE]=1;
  vc_route_setup(&s);vc_note_on(&s,60,127);VCVoice *v=&s.voice[0];v->lfo.value=values[j];vc_filter_control(&s,v);
  emit(v->filter[0].resonance);emit(v->filter[1].resonance);emit(v->filter[0].k);emit(v->filter[1].k);emit(v->lfo_filter_gain);
  for(unsigned n=0;n<64;n++){float l,r;vc_route_sample(&s,v,(n%31-15.f)/128,(n%23-11.f)/128,&l,&r);emit(l);emit(r);}
  s.param[VC_LFO_MODE]=0;vc_filter_control(&s,v);assert(v->lfo_filter_gain==1&&v->filter[0].resonance==55&&v->filter[1].resonance==70);
 }
}
