#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
#include "hold-mode-events.inc"
static void emit(unsigned x){fwrite(&x,4,1,stdout);}
int main(void){
 VCSynth s;vc_init(&s,44100);vc_param(&s,VC_KEY_MODE,5);
 for(unsigned i=0;i<sizeof(events)/sizeof(events[0]);i++){
  for(int k=0;k<VC_VOICES;k++)if(!s.voice[k].held)s.voice[k].env.stage=0;
  int t=events[i][0],n=events[i][1],vel=events[i][2];
  if(t==0)vc_note_on(&s,n,vel);else if(t==1)vc_note_off(&s,n);else if(t==2)vc_cc(&s,64,n);else if(t==3)vc_cc(&s,123,0);
  assert(s.mono_count<=128&&s.mono_deferred_count<=128&&vc_active(&s)<=4);
  for(int n=0;n<128;n++){
   unsigned down=0,deferred=0,held=0;
   for(unsigned j=0;j<s.mono_count;j++)down|=s.mono_notes[j]==n;
   for(unsigned j=0;j<s.mono_deferred_count;j++)deferred|=s.mono_deferred[j]==n;
   for(int j=0;j<VC_VOICES;j++)held|=s.voice[j].env.stage&&s.voice[j].held&&s.voice[j].note==n;
   emit(down);emit(deferred);emit(!!(s.hold_latched[n>>3]&(1u<<(n&7))));emit(held);
  }
  emit(s.pedal);
 }
 vc_panic(&s);assert(!vc_active(&s)&&!s.mono_count&&!s.mono_deferred_count&&!s.pedal);
 for(int k=0;k<16;k++)assert(!s.hold_latched[k]);
 vc_note_on(&s,60,100);vc_note_off(&s,60);vc_param(&s,VC_KEY_MODE,0);assert(!vc_active(&s)&&!s.mono_count);
 for(int k=0;k<16;k++)assert(!s.hold_latched[k]);
 /* Real render: latched voices survive releases and block splitting agrees. */
 vc_param(&s,VC_KEY_MODE,5);vc_note_on(&s,60,100);vc_note_on(&s,64,90);vc_note_on(&s,67,80);vc_note_off(&s,60);vc_note_off(&s,64);vc_note_off(&s,67);
 VCSynth other=s;float a[256],b[256],ar[256],br[256];
 for(int j=0;j<100;j++){vc_render_stereo(&s,a,ar,256);for(int k=0;k<256;k++)vc_render_stereo(&other,b+k,br+k,1);for(int k=0;k<256;k++){assert(__builtin_isfinite(a[k])&&__builtin_isfinite(ar[k]));assert(a[k]==b[k]&&ar[k]==br[k]);}}
 assert(vc_active(&s)==3);vc_note_on(&s,72,100);unsigned held=0;for(int k=0;k<4;k++)held+=s.voice[k].held;assert(held==1);
 return 0;
}
