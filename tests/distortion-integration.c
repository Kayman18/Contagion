#include "virooos.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
int main(void){
 for(unsigned mode=0;mode<=11;mode++){
  VCSynth dry,wet;VCDistortion fx;vc_init(&dry,44100);vc_init(&wet,44100);vc_distortion_reset(&fx);
  vc_param(&wet,VC_DIST_MODE,mode);vc_param(&wet,VC_DIST_LEVEL,63);vc_distortion_set(&fx,mode,63);
  for(unsigned n=0;n<4;n++){vc_note_on(&dry,48+4*n,100);vc_note_on(&wet,48+4*n,100);}
  for(unsigned block=0;block<32;block++){
   if(block==16){vc_param(&wet,VC_DIST_LEVEL,127);vc_distortion_set(&fx,mode,127);}
   float dl[128],dr[128],wl[128],wr[128];vc_render_stereo(&dry,dl,dr,128);vc_render_stereo(&wet,wl,wr,128);
   for(unsigned i=0;i<128;i++){float l=vc_distortion_next(&fx,dl[i],0),r=vc_distortion_next(&fx,dr[i],1);assert(isfinite(wl[i])&&isfinite(wr[i]));assert(l==wl[i]&&r==wr[i]);}
  }
 }
 puts("PASS: all 12 settings, four voices, stereo post-mix placement, live intensity changes; 98304 output samples exact to separate DSP chain");
}
