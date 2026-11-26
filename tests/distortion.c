#include "distortion.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
int main(void){
 VCDistortion s;unsigned mode,level,ch;float x;
 while(scanf("%u %u %u %f",&mode,&level,&ch,&x)==4){
  if(ch==2){vc_distortion_reset(&s);vc_distortion_set(&s,mode,level);continue;}
  if(ch==3){vc_distortion_set(&s,mode,level);continue;}
  float y=vc_distortion_next(&s,x,ch);assert(isfinite(y));fwrite(&y,sizeof(y),1,stdout);
 }
 return 0;
}
