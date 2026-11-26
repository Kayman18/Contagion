#include "virooos.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
int main(void){
 const float steps[]={-.25f,-.0078125f,0,.001953125f,.125f,.75f,2.25f};
 const unsigned shapes[]={0,32,64,127};
 for(unsigned w=0;w<4;w+=3)for(unsigned s=0;s<4;s++)for(unsigned width=0;width<2;width++)for(unsigned sync=0;sync<2;sync++){
  VCOsc o={0};vc_osc_setup(&o,w,shapes[s],width?.5f:.25f,1.f/128);
  for(unsigned n=0;n<512;n++){float x=vc_osc_next(&o,steps[n%7],sync&&n%11==0?.375f:-1);assert(isfinite(x));fwrite(&x,4,1,stdout);}
 }
}
