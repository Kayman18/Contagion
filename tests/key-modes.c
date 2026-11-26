#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
#include "key-modes-events.inc"
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 for(unsigned mode=1;mode<=4;mode++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_KEY_MODE,mode);vc_param(&s,VC_PORTAMENTO,127);
  for(unsigned i=0;i<sizeof(events)/sizeof(events[0]);i++){
   VCVoice *v=&s.voice[0];if(v->env.stage)v->pitch+=(v->note-v->pitch)*.1f;
   int t=events[i][0],n=events[i][1],vel=events[i][2];
   if(t==0)vc_note_on(&s,n,vel);else if(t==1)vc_note_off(&s,n);else if(t==2)vc_cc(&s,64,n);else if(t==3)vc_cc(&s,123,0);
   else if(t==4&&v->env.stage&&v->held){v->env.stage=v->filter_env.stage=3;v->env.level=v->filter_env.level=.4f;}
   else if(t==5&&v->env.stage&&!v->held){v->env.stage=v->filter_env.stage=0;}
   unsigned active=vc_active(&s);assert(active<=1&&s.mono_count<=128&&s.mono_deferred_count<=128);
   emit(active);emit(active?v->note:-1);emit(active?v->held:0);emit(active?v->pitch:0);emit(active?v->velocity:0);emit(active?v->env.stage:0);emit(active?v->filter_env.stage:0);emit(s.mono_count);emit(s.last_note);emit(s.pedal);
  }
 }
 /* Poly default and mode transitions, and zero-velocity note-off. */
 VCSynth s;vc_init(&s,44100);for(int i=0;i<4;i++)vc_note_on(&s,60+i,100);assert(vc_active(&s)==4);
 vc_param(&s,VC_KEY_MODE,3);assert(vc_active(&s)==0&&s.mono_count==0);
 vc_note_on(&s,60,100);vc_note_on(&s,64,80);vc_note_on(&s,64,0);assert(s.voice[0].note==60&&s.mono_count==1);
 vc_panic(&s);assert(!vc_active(&s)&&!s.mono_count&&!s.mono_deferred_count&&!s.pedal);
 vc_param(&s,VC_KEY_MODE,0);for(int i=0;i<4;i++)vc_note_on(&s,60+i,100);assert(vc_active(&s)==4);
 for(unsigned mode=1;mode<=4;mode++){
  VCSynth a,b;vc_init(&a,44100);vc_param(&a,VC_KEY_MODE,mode);vc_param(&a,VC_PORTAMENTO,96);vc_param(&a,VC_LFO_MODE,1);vc_param(&a,VC_LFO_OSC2,80);b=a;
  for(int step=0;step<32;step++){
   int note=48+step%12;vc_note_on(&a,note,100);vc_note_on(&b,note,100);
   float l[256],r[256],ll[256],rr[256];vc_render_stereo(&a,l,r,256);
   for(unsigned j=0;j<256;j++)vc_render_stereo(&b,ll+j,rr+j,1);
   assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));assert(vc_active(&a)==1);
   for(unsigned j=0;j<256;j++)assert(__builtin_isfinite(l[j])&&__builtin_isfinite(r[j]));
   VCVoice *v=&a.voice[0];v->env.stage=v->filter_env.stage=3;
   float phase=v->osc[0].phase,level=v->env.level,filter=v->filter[0].s1;
   vc_note_on(&a,note+12,80);assert(v->osc[0].phase==phase&&v->env.level==level&&v->filter[0].s1==filter);
   assert(v->env.stage==(mode<=2?1:3));
   b=a;
   vc_note_off(&a,note+12);vc_note_off(&b,note+12);
   assert(a.voice[0].note==note);
  }
 }

}
