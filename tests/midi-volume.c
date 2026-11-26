#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 for(int a=0;a<128;a++)for(int b=0;b<128;b++){
  VCSynth s;vc_init(&s,44100);vc_cc(&s,7,a);vc_cc(&s,11,b);vc_note_on(&s,60,127);emit(s.voice[0].amp_gain);
 }
 VCSynth s;vc_init(&s,44100);for(int i=0;i<4;i++)vc_note_on(&s,48+i*7,127);
 float l[64],r[64];vc_render_stereo(&s,l,r,64);
 VCVoice before[4];memcpy(before,s.voice,sizeof before);vc_cc(&s,7,64);vc_cc(&s,11,96);
 for(int i=0;i<4;i++){
  VCVoice *v=&s.voice[i];assert(v->env.level==before[i].env.level&&v->env.stage==before[i].env.stage);
  assert(v->osc[0].phase==before[i].osc[0].phase&&v->lfo.phase==before[i].lfo.phase);
  for(int j=0;j<32;j++){emit(vc_amp_gain_next(v));}assert(v->amp_remaining==0);
 }
 vc_note_off(&s,48);assert(s.voice[0].env.stage);vc_cc(&s,11,0);
 vc_render_stereo(&s,l,r,64);for(int i=32;i<64;i++)assert(l[i]==0&&r[i]==0);
 assert(vc_active(&s)>0);vc_panic(&s);assert(s.expression==0);vc_note_on(&s,60,127);assert(s.voice[0].amp_gain==0);
 vc_cc(&s,7,999);vc_cc(&s,11,999);assert(s.channel_volume==1&&s.expression==1);
 for(int j=0;j<32;j++){vc_amp_gain_next(&s.voice[0]);}assert(s.voice[0].amp_gain==1);
 vc_cc(&s,7,-1);assert(s.channel_volume==0);vc_init(&s,44100);assert(s.channel_volume==1&&s.expression==1);
}
