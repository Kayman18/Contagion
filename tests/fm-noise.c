#include "virooos.h"
#include <assert.h>
#include <stdio.h>
int main(void){
 for(unsigned mode=0;mode<4;mode++)for(unsigned audible=0;audible<2;audible++)for(unsigned amount=0;amount<2;amount++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_FM_MODE,mode);vc_param(&s,VC_FM_AMOUNT,amount?64:0);vc_param(&s,VC_NOISE_VOLUME,audible?64:0);vc_param(&s,VC_NOISE_COLOR,0);vc_note_on(&s,60,100);
  VCNoise expected=s.voice[0].noise;float l[256],r[256];
  if(audible||(mode==3&&amount))for(unsigned n=0;n<256;n++)vc_noise_next(&expected);
  vc_render_stereo(&s,l,r,256);assert(s.voice[0].noise.rng==expected.rng&&s.voice[0].noise.lp==expected.lp);
 }
 puts("PASS: audible noise and noise-FM share exactly one generator step per sample; unused noise pauses");
}
