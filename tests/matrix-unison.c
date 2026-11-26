#include <stdio.h>
#include <assert.h>
#include "../engine/virooos.c"
int main(void){
 for(unsigned count=2;count<=4;count++)for(unsigned amount=0;amount<128;amount++)for(unsigned test=0;test<4;test++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_UNISON_MODE,count-1);vc_param(&s,VC_UNISON_DETUNE,64);
  vc_param(&s,VC_OSC3_MODE,4);VCMatrix m={0};m.route[0]=(VCMatrixRoute){3,59,amount};
  vc_set_matrix(&s,&m);vc_note_on(&s,60,100);s.controllers[3]=(float[]){-1,0,.37f,1}[test];
  for(unsigned i=0;i<count;i++){
   VCVoice *v=&s.voice[i];v->osc[0].phase=.31f;v->osc[1].phase=.29f;v->osc3.phase=.17f;v->osc[0].pending=.007f;
   vc_filter_control(&s,v);
   assert(v->osc[0].phase==.31f&&v->osc[1].phase==.29f&&v->osc3.phase==.17f&&v->osc[0].pending==.007f);
   printf("%u %u %u %u %.9g %.9g %.9g %.9g\n",count,amount,test,i,v->unison_cents,v->inc[0],v->inc[1],v->inc3);
  }
  vc_set_matrix(&s,0);for(unsigned i=0;i<count;i++){vc_filter_control(&s,&s.voice[i]);float want=s.voice[i].unison_side==0?0:s.voice[i].unison_side*vc_unison_detune[64]+(count&1?2.37f:0);assert(fabsf(want-s.voice[i].unison_cents)<.00001f);}
 }
 /* No unison: target is inert; odd-stack center remains exactly centered. */
 VCSynth s;vc_init(&s,44100);vc_note_on(&s,60,100);float inc=s.voice[0].inc[0];
 VCMatrix m={0};m.route[0]=(VCMatrixRoute){3,59,127};vc_set_matrix(&s,&m);s.controllers[3]=1;vc_filter_control(&s,&s.voice[0]);assert(s.voice[0].inc[0]==inc);
}
