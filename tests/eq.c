#include "eq.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
int main(void){VCEQ s;char command;while(scanf(" %c",&command)==1){if(command=='Q'||command=='U'){unsigned p[7];for(unsigned i=0;i<7;i++)assert(scanf("%u",&p[i])==1);if(command=='Q')vc_eq_reset(&s);vc_eq_set(&s,p[0],p[1],p[2],p[3],p[4],p[5],p[6]);fwrite(s.mid,4,5,stdout);fwrite(s.low,4,3,stdout);fwrite(s.high,4,3,stdout);fwrite(&s.normalization,4,1,stdout);}else{unsigned ch;float x;assert(scanf("%u %f",&ch,&x)==2);float y=vc_eq_next(&s,x,ch);assert(isfinite(y));fwrite(&y,4,1,stdout);}}}
