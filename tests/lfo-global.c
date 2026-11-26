#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void render(VCSynth *s,unsigned n){float l[256],r[256];assert(n<=256);vc_render_stereo(s,l,r,n);for(unsigned i=0;i<n;i++)assert(__builtin_isfinite(l[i])&&__builtin_isfinite(r[i]));}
int main(void){
 for(unsigned k=0;k<2;k++){
  unsigned id=k?VC_LFO2_GLOBAL:VC_LFO_GLOBAL,mode=k?VC_LFO2_MODE:VC_LFO_MODE,trigger=k?VC_LFO2_TRIGGER:VC_LFO_TRIGGER;
  for(int x=-1;x<=128;x++){VCSynth s;vc_init(&s,44100);vc_cc(&s,k?82:70,x);assert(s.param[id]==(x>0));assert(s.param[mode]==0);}
  VCSynth s;vc_init(&s,44100);vc_param(&s,id,1);vc_param(&s,mode,1);vc_param(&s,trigger,0);render(&s,256);
  float phase=s.global_lfo[k].phase;vc_note_on(&s,48,100);assert(s.global_lfo[k].phase==phase);render(&s,32);phase=s.global_lfo[k].phase;vc_note_on(&s,72,100);assert(s.global_lfo[k].phase==phase);
  render(&s,32);assert(k?s.voice[0].lfo2_mod==s.voice[1].lfo2_mod:s.voice[0].lfo_mod==s.voice[1].lfo_mod);
  float rate=s.global_lfo[k].rate;vc_param(&s,k?VC_LFO2_KEYTRACK:VC_LFO_KEYTRACK,127);assert(s.global_lfo[k].rate==rate);
  phase=s.global_lfo[k].phase;vc_param(&s,id,0);render(&s,32);assert(s.global_lfo[k].phase!=phase);phase=s.global_lfo[k].phase;vc_param(&s,id,1);assert(s.global_lfo[k].phase==phase);
  for(unsigned keymode=0;keymode<6;keymode++)for(unsigned t=0;t<128;t++){
   vc_param(&s,VC_KEY_MODE,keymode);vc_param(&s,trigger,t);vc_param(&s,mode,1);render(&s,32);phase=s.global_lfo[k].phase;
   vc_note_on(&s,60,100);assert(s.global_lfo[k].phase==(t?(t-1)/128.f:phase));
   render(&s,32);vc_note_on(&s,64,100);if(t)assert(s.global_lfo[k].phase==(t-1)/128.f);
   phase=s.global_lfo[k].phase;vc_note_on(&s,67,0);vc_note_on(&s,128,100);assert(s.global_lfo[k].phase==phase);
  }
  vc_param(&s,mode,2);vc_param(&s,trigger,0);vc_note_on(&s,60,100);assert(s.global_lfo[k].phase==0);
  vc_lfo_tick(&s.global_lfo[k],441000);assert(s.global_lfo[k].done);vc_note_on(&s,64,100);assert(!s.global_lfo[k].done&&s.global_lfo[k].phase==0);
  vc_lfo_tick(&s.global_lfo[k],441000);vc_cc(&s,k?81:69,0);assert(!s.global_lfo[k].done);
 }
 /* Deterministic source comparison, including irregular render boundaries,
  * clocked rate, retrigger, completed envelope and all deterministic shapes. */
 unsigned sizes[]={7,24,1,65,31,128,17,256};
 for(unsigned shape=0;shape<=67;shape++){if(shape==4||shape==5)continue;
  for(unsigned env=0;env<2;env++){
   VCSynth s;vc_init(&s,44100);
   for(unsigned k=0;k<2;k++){
    vc_param(&s,k?VC_LFO2_GLOBAL:VC_LFO_GLOBAL,1);vc_param(&s,k?VC_LFO2_MODE:VC_LFO_MODE,1+env);
    vc_param(&s,k?VC_LFO2_SHAPE:VC_LFO_SHAPE,shape);vc_param(&s,k?VC_LFO2_RATE:VC_LFO_RATE,100);
    vc_param(&s,k?VC_LFO2_CONTOUR:VC_LFO_CONTOUR,80);vc_param(&s,k?VC_LFO2_TRIGGER:VC_LFO_TRIGGER,33);
   }
   vc_param(&s,VC_LFO2_CLOCK,1);vc_note_on(&s,60,100);
   for(unsigned step=0;step<160;step++){
    if(step==80)vc_note_on(&s,72,100);
    render(&s,sizes[step%8]);
    for(unsigned k=0;k<2;k++)printf("%u %u %u %u %.9g %.9g %u\n",shape,env,step,k,s.global_lfo[k].phase,s.global_lfo[k].value,s.global_lfo[k].done);
   }
  }
 }
 return 0;
}
