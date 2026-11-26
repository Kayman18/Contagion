#include "virooos.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static float max_sample;static VCSynth s;static float out[256];
static void render(int blocks){for(int n=0;n<blocks;n++){vc_render(&s,out,256);for(int j=0;j<256;j++){assert(isfinite(out[j]));if(fabsf(out[j])>max_sample)max_sample=fabsf(out[j]);}}}
#ifdef VC_DYNAMIC_WAVES
#include "wave_cache.h"
#endif
int main(void){
#ifdef VC_DYNAMIC_WAVES
 assert(vc_wave_init(vc_wave_seed(),vc_wave_seed_size()));
 for(unsigned w=0;w<2;w++){assert(vc_wave_begin(w,w));int state=0;while(!state)state=vc_wave_service(4096);assert(state==1);}
#endif
 vc_init(&s,44100);assert(vc_active(&s)==0);render(1);for(int i=0;i<256;i++)assert(out[i]==0);
 for(int n=60;n<64;n++)vc_note_on(&s,n,100);assert(vc_active(&s)==4);render(30);float power=0;for(int i=0;i<256;i++)power+=out[i]*out[i];assert(power>.0001f);
 vc_note_on(&s,72,100);assert(vc_active(&s)==4);render(10);
 vc_cc(&s,64,127);vc_cc(&s,123,0);render(10);assert(vc_active(&s)==4);vc_cc(&s,64,0);render(3000);assert(vc_active(&s)==0);
 vc_note_on(&s,64,100);vc_note_on(&s,64,0);render(3000);assert(vc_active(&s)==0);
 vc_note_on(&s,64,100);vc_cc(&s,120,0);assert(vc_active(&s)==0);render(1);
 for(int mode=0;mode<4;mode++)for(int poles=0;poles<4;poles++)for(int cut=0;cut<=128;cut+=8){vc_init(&s,44100);vc_param(&s,VC_FILTER_MODE,mode);vc_param(&s,VC_ROUTING,poles);vc_param(&s,VC_CUTOFF,cut>127?127:cut);vc_param(&s,VC_CUTOFF2,cut>127?127:cut);vc_param(&s,VC_RESONANCE,127);vc_param(&s,VC_RESONANCE2,127);vc_param(&s,VC_FILTER_MODE2,mode);vc_param(&s,VC_TUNE1,16+mode*32);vc_param(&s,VC_TUNE2,112-mode*32);vc_param(&s,VC_KEYFOLLOW1,poles*32);vc_param(&s,VC_PW1,cut);vc_param(&s,VC_PW2,127-cut);vc_param(&s,VC_LFO_MODE,1+(mode&1));vc_param(&s,VC_LFO_SHAPE,(cut/8)%6);vc_param(&s,VC_LFO_CONTOUR,cut>127?127:cut);vc_param(&s,VC_VEL_ENV1,mode*42);vc_param(&s,VC_VEL_ENV2,127-mode*42);vc_param(&s,VC_VEL_RES1,poles*42);vc_param(&s,VC_VEL_RES2,127-poles*42);vc_param(&s,VC_VEL_SHAPE1,mode*42);vc_param(&s,VC_VEL_SHAPE2,127-mode*42);vc_param(&s,VC_VEL_PW,poles*42);vc_param(&s,VC_VEL_PAN,127-poles*42);vc_param(&s,VC_AMP_VELOCITY,mode*42);vc_param(&s,VC_TRANSPOSE,mode*42);vc_param(&s,VC_BEND_UP,mode*42);vc_param(&s,VC_BEND_DOWN,127-mode*42);vc_param(&s,VC_BEND_SCALE,mode&1);vc_param(&s,VC_PORTAMENTO,mode*42);vc_param(&s,VC_PUNCH,cut>127?127:cut);vc_param(&s,VC_SAT_CURVE,(cut/8)%15);vc_param(&s,VC_SAT_DRIVE,mode*42);vc_param(&s,VC_OSC3_MODE,1+(cut/8)%3);vc_param(&s,VC_OSC3_VOLUME,100);vc_param(&s,VC_OSC3_TUNE,mode*32+16);vc_param(&s,VC_LFO3_ENABLE,1);vc_param(&s,VC_LFO3_RATE,127);vc_param(&s,VC_LFO3_AMOUNT,127);vc_param(&s,VC_LFO3_DEST,(cut/8)%6);vc_param(&s,VC_LFO3_SHAPE,(cut/8)%6);vc_param(&s,VC_LFO3_FADE,mode*42);vc_param(&s,VC_LFO2_PAN,mode*42);vc_param(&s,VC_LFO2_SHAPE_AMOUNT,mode*42);vc_param(&s,VC_LFO2_FM_AMOUNT,poles*42);vc_param(&s,VC_LFO2_CLOCK,(cut/8)%22);vc_param(&s,VC_LFO2_RATE,127);vc_param(&s,VC_LFO2_MODE,1+(mode&1));vc_param(&s,VC_LFO2_SHAPE,(cut/8)%6);vc_param(&s,VC_LFO2_CONTOUR,cut>127?127:cut);vc_param(&s,VC_LFO2_CUTOFF1,mode*42);vc_param(&s,VC_LFO2_CUTOFF2,poles*42);vc_param(&s,VC_LFO2_KEYTRACK,127);vc_param(&s,VC_LFO_RATE,127);vc_param(&s,VC_LFO_CLOCK,poles?1+cut%21:0);vc_param(&s,VC_LFO_KEYTRACK,127);vc_param(&s,VC_TEMPO,mode*42);vc_param(&s,VC_LFO_RESONANCE,cut>127?127:cut);vc_param(&s,VC_LFO_FILTER_GAIN,mode*42);vc_param(&s,VC_LFO_OSC1,mode*42);vc_param(&s,VC_LFO_OSC2,poles*32);vc_param(&s,VC_LFO_PW,127-cut);vc_param(&s,VC_OSC2_ENV,cut>127?127:cut);vc_param(&s,VC_FM_AMOUNT,mode*42);vc_param(&s,VC_FM_MODE,poles);vc_param(&s,VC_FM_ENV,cut>127?127:cut);vc_param(&s,VC_FM_VELOCITY,mode*42);vc_param(&s,VC_SYNC,poles&1);vc_param(&s,VC_NOISE_VOLUME,90);vc_param(&s,VC_NOISE_COLOR,mode*42);vc_param(&s,VC_SUB_VOLUME,90);vc_param(&s,VC_SUB_SHAPE,mode&1);vc_param(&s,VC_RING_VOLUME,70);vc_param(&s,VC_F_AMOUNT1,60);vc_param(&s,VC_F_AMOUNT2,80);for(int i=0;i<4;i++)vc_note_on(&s,24+i*24,127);render(100);vc_bend(&s,16383);render(10);vc_bend(&s,0);render(10);}
 vc_panic(&s);vc_param(&s,VC_FILTER_MODE,999);assert(s.param[VC_FILTER_MODE]==3);vc_param(&s,999,0);assert(vc_active(&s)==0);
 {
  VCSynth a,b;float al[1536],ar[1536],bl[1536],br[1536];
  vc_init(&a,44100);vc_param(&a,VC_F_AMOUNT1,80);vc_param(&a,VC_F_AMOUNT2,64);vc_param(&a,VC_ROUTING,3);vc_param(&a,VC_SPREAD,127);
  vc_param(&a,VC_LFO_RESONANCE,100);vc_param(&a,VC_LFO_FILTER_GAIN,110);vc_param(&a,VC_LFO_CLOCK,11);vc_param(&a,VC_TEMPO,90);vc_param(&a,VC_LFO_KEYTRACK,100);vc_param(&a,VC_VEL_ENV1,96);vc_param(&a,VC_VEL_ENV2,32);vc_param(&a,VC_VEL_RES1,110);vc_param(&a,VC_VEL_RES2,24);vc_param(&a,VC_VEL_SHAPE1,96);vc_param(&a,VC_VEL_SHAPE2,32);vc_param(&a,VC_VEL_PW,110);vc_param(&a,VC_VEL_PAN,24);vc_param(&a,VC_AMP_VELOCITY,96);vc_param(&a,VC_TRANSPOSE,76);vc_param(&a,VC_BEND_UP,76);vc_param(&a,VC_BEND_DOWN,40);vc_param(&a,VC_BEND_SCALE,1);vc_param(&a,VC_PORTAMENTO,80);vc_param(&a,VC_PUNCH,96);vc_param(&a,VC_SAT_CURVE,10);vc_param(&a,VC_SAT_DRIVE,32);vc_param(&a,VC_OSC3_MODE,1);vc_param(&a,VC_LFO3_ENABLE,1);vc_param(&a,VC_LFO3_AMOUNT,80);vc_param(&a,VC_LFO3_DEST,4);vc_param(&a,VC_LFO3_FADE,40);vc_param(&a,VC_LFO2_PAN,110);vc_param(&a,VC_LFO2_SHAPE_AMOUNT,110);vc_param(&a,VC_LFO2_FM_AMOUNT,100);vc_param(&a,VC_LFO2_CLOCK,5);vc_param(&a,VC_LFO2_MODE,1);vc_param(&a,VC_LFO2_SHAPE,5);vc_param(&a,VC_LFO2_CUTOFF1,110);vc_param(&a,VC_LFO2_CUTOFF2,20);vc_param(&a,VC_LFO_MODE,1);vc_param(&a,VC_LFO_SHAPE,5);vc_param(&a,VC_LFO_OSC1,90);vc_param(&a,VC_LFO_OSC2,95);vc_param(&a,VC_LFO_PW,100);vc_param(&a,VC_OSC2_ENV,110);vc_param(&a,VC_FM_AMOUNT,32);vc_param(&a,VC_FM_MODE,3);vc_param(&a,VC_FM_ENV,90);vc_param(&a,VC_NOISE_VOLUME,90);vc_param(&a,VC_NOISE_COLOR,23);vc_param(&a,VC_SUB_VOLUME,90);vc_param(&a,VC_RING_VOLUME,70);vc_note_on(&a,60,100);vc_note_on(&a,67,100);b=a;
  vc_render_stereo(&a,al,ar,1536);
  unsigned chunks[]={1,3,17,32,65,256},at=0,i=0;
  while(at<1536){unsigned n=chunks[i++%6];if(n>1536-at)n=1536-at;vc_render_stereo(&b,bl+at,br+at,n);at+=n;}
  assert(!memcmp(al,bl,sizeof(al))&&!memcmp(ar,br,sizeof(ar)));
  float diff=0;for(int j=0;j<1536;j++){assert(isfinite(al[j])&&isfinite(ar[j]));diff+=fabsf(al[j]-ar[j]);}assert(diff>.1f);
  vc_init(&a,44100);vc_param(&a,VC_PAN,0);vc_note_on(&a,60,100);vc_render_stereo(&a,al,ar,1536);
  for(int j=0;j<1536;j++)assert(ar[j]==0);
  vc_param(&a,VC_PAN,127);vc_render_stereo(&a,al,ar,1536);for(int j=0;j<1536;j++)assert(al[j]==0);
 }
 printf("PASS: voice limit, stealing, sustain, zero-velocity note-off, panic, parameter bounds, filter/envelope stress, stereo isolation, block invariance; synth RAM %zu bytes\n",sizeof(s));printf("Maximum raw stress sample: %f\n",max_sample);
}
