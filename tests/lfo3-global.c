#include "../engine/virooos.c"
#include <stdio.h>
#include <assert.h>
int main(void){const unsigned shapes[]={0,1,2,3,6,45,67};const unsigned sizes[]={7,24,1,65,31,128,17,256};
 for(unsigned a=0;a<7;a++)for(unsigned clock=0;clock<=5;clock+=5){VCSynth s;vc_init(&s,44100);vc_param(&s,VC_LFO3_GLOBAL,1);vc_param(&s,VC_LFO3_ENABLE,1);vc_param(&s,VC_LFO3_AMOUNT,100);vc_param(&s,VC_LFO3_RATE,92);vc_param(&s,VC_LFO3_SHAPE,shapes[a]);vc_param(&s,VC_LFO3_CLOCK,clock);s.global_lfo[2].phase=.25f;
  for(unsigned step=0;step<200;step++){if(step==80)vc_note_on(&s,60,100);if(step==120)vc_note_on(&s,72,100);unsigned n=sizes[step%8];float l[256],r[256];vc_render_stereo(&s,l,r,n);
   if(step>=80){vc_filter_control(&s,&s.voice[0]);assert(s.voice[0].lfo3_mod==s.global_lfo[2].value);}
   printf("%u %u %u %.9g %.9g\n",shapes[a],clock,step,s.global_lfo[2].phase,s.global_lfo[2].value);
  }
 }
}
