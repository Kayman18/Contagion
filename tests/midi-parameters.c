#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
int main(void){
 const int cc[]={5,10,17,18,20,21,22,23,25,26,27,28,29,30,31,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,51,52,53,54,55,56,57,58,59,60,61,62,63,67,68,72,73,75,76,77,78,79,80,83,84,85,86,87,88,89,90,91,93,71,74},ids[]={VC_PORTAMENTO,VC_PAN,VC_SHAPE1,VC_PW1,VC_TUNE1,VC_KEYFOLLOW1,VC_SHAPE2,VC_PW2,VC_TUNE2,VC_DETUNE,VC_FM_AMOUNT,VC_SYNC,VC_OSC2_ENV,VC_FM_ENV,VC_KEYFOLLOW2,VC_BALANCE,VC_SUB_VOLUME,VC_SUB_SHAPE,VC_SAT_DRIVE,VC_NOISE_VOLUME,VC_RING_VOLUME,VC_NOISE_COLOR,VC_CUTOFF,VC_CUTOFF2,VC_RESONANCE,VC_RESONANCE2,VC_F_AMOUNT1,VC_F_AMOUNT2,VC_F_KEYFOLLOW1,VC_F_KEYFOLLOW2,VC_FILTER_BALANCE,VC_SAT_CURVE,VC_FILTER_MODE,VC_FILTER_MODE2,VC_ROUTING,VC_F_ATTACK,VC_F_DECAY,VC_F_SUSTAIN,VC_F_SLOPE,VC_F_RELEASE,VC_ATTACK,VC_DECAY,VC_SUSTAIN,VC_AMP_SLOPE,VC_RELEASE,VC_LFO_RATE,VC_LFO_SHAPE,VC_LFO_KEYTRACK,VC_LFO_TRIGGER,VC_LFO_OSC2,VC_LFO_PW,VC_LFO_RESONANCE,VC_LFO_FILTER_GAIN,VC_LFO2_RATE,VC_LFO2_SHAPE,VC_LFO2_CONTOUR,VC_LFO2_KEYTRACK,VC_LFO2_TRIGGER,VC_LFO2_SHAPE_AMOUNT,VC_LFO2_FM_AMOUNT,VC_LFO2_CUTOFF1,VC_LFO2_CUTOFF2,VC_LFO2_PAN,VC_PATCH_VOLUME,VC_TRANSPOSE,VC_RESONANCE,VC_CUTOFF};
 for(unsigned c=0;c<sizeof(cc)/sizeof(cc[0]);c++)for(unsigned x=0;x<128;x++)for(unsigned mode=0;mode<3;mode++){
  VCSynth a,b;vc_init(&a,44100);vc_init(&b,44100);vc_param(&a,VC_LFO_MODE,mode);vc_param(&b,VC_LFO_MODE,mode);vc_param(&a,VC_LFO2_MODE,mode);vc_param(&b,VC_LFO2_MODE,mode);vc_note_on(&a,48,100);vc_note_on(&b,48,100);
  assert(vc_cc_param(cc[c])==ids[c]);vc_cc(&a,cc[c],x);vc_param(&b,ids[c],x);
  assert(!memcmp(&a,&b,sizeof a));float l[64],r[64],ll[64],rr[64];vc_render_stereo(&a,l,r,64);vc_render_stereo(&b,ll,rr,64);assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));
  vc_cc(&a,cc[c],-1);assert(a.param[ids[c]]==0);vc_cc(&a,cc[c],200);assert(a.param[ids[c]]==vc_param_max(ids[c]));
 }
 assert(vc_cc_param(19)==-1&&vc_cc_param(24)==-1);
 puts("25728 MIDI/direct-parameter state and stereo-render comparisons passed; clamping passed");
}
