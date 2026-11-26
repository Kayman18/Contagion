#include "virooos.h"
#include <stdio.h>
int main(void){
 for(int shape=0;shape<2;shape++)for(int rate=0;rate<4;rate++)for(int phase=0;phase<4;phase++){
  VCSub s={phase*.125f};float inc=(1<<rate)/1024.f;
  for(int j=0;j<4096;j++){float x=vc_sub_next(&s,inc,shape);fwrite(&x,4,1,stdout);}
 }
}
