#include "virooos.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 VCSynth a,b;float l[256],r[256];
 vc_profile_prepare(&a);assert(vc_active(&a)==4);assert(a.param[VC_SUB_VOLUME]==17&&a.param[VC_RING_VOLUME]==13);
 b=a;vc_profile_run(&a,0,256);vc_render_stereo(&b,l,r,256);assert(!memcmp(&a,&b,sizeof(a)));
 vc_profile_prepare(&a);vc_profile_run(&a,1,256);assert(a.voice[0].osc[0].phase>0&&a.voice[0].sub.phase>0);assert(!a.voice[0].filter[0].configured);
 vc_profile_prepare(&a);vc_profile_run(&a,2,256);assert(a.voice[0].filter[0].configured&&a.voice[0].filter[0].s1!=0);assert(a.voice[0].osc[0].phase==0);
 vc_profile_prepare(&a);a.voice[0].env.stage=1;a.voice[0].env.level=0;vc_profile_run(&a,3,256);assert(a.voice[0].env.level>0);assert(!a.voice[0].filter[0].configured);
 vc_profile_prepare(&a);vc_profile_run(&a,4,256);assert(a.voice[0].filter[0].configured);assert(a.voice[0].control_remaining==0);assert(a.voice[0].osc[0].phase==0);
 vc_profile_prepare(&a);b=a;vc_profile_run(&a,5,256);assert(!memcmp(&a,&b,sizeof(a)));
 for(unsigned mode=0;mode<6;mode++){vc_profile_prepare(&a);for(int j=0;j<200;j++)vc_profile_run(&a,mode,256);assert(vc_active(&a)==4);}
 puts("PASS: profiler full-render equivalence, stage isolation, fixed four-note setup, idle state preservation, repeated workloads");
}
