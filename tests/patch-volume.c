#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 for(unsigned a=0;a<128;a++)for(unsigned c=0;c<4;c++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_PATCH_VOLUME,a);vc_cc(&s,7,c*42);vc_cc(&s,11,96);vc_note_on(&s,60,100);emit(s.voice[0].amp_gain);
 }
 for(unsigned a=0;a<128;a++){
  VCSynth s;vc_init(&s,44100);vc_note_on(&s,60,127);VCVoice *v=&s.voice[0];float l[64],r[64];vc_render_stereo(&s,l,r,64);
  VCVoice before=*v;vc_param(&s,VC_PATCH_VOLUME,a);assert(!memcmp(&before.osc,&v->osc,sizeof v->osc));assert(!memcmp(&before.env,&v->env,sizeof v->env));assert(!memcmp(&before.lfo,&v->lfo,sizeof v->lfo));
  for(unsigned j=0;j<40;j++)emit(vc_amp_gain_next(v));
  vc_param(&s,VC_PATCH_VOLUME,0);vc_render_stereo(&s,l,r,64);for(unsigned j=32;j<64;j++)assert(l[j]==0&&r[j]==0);
  vc_param(&s,VC_PATCH_VOLUME,100);for(unsigned j=0;j<32;j++){vc_amp_gain_next(v);}assert(v->amp_gain==1);
 }
}
