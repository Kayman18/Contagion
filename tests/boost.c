#include "../engine/eq.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
static void emit(float v){fwrite(&v,4,1,stdout);}
int main(void){
 VCBoost s={0};unsigned intensity,tune,frames,reset;uint32_t seed=12345;
 while(scanf("%u%u%u%u",&intensity,&tune,&frames,&reset)==4){
  if(reset)memset(&s,0,sizeof s);
  vc_boost_set(&s,intensity,tune);for(unsigned j=0;j<3;j++)emit(s.coef[j]);
  for(unsigned i=0;i<frames;i++)for(unsigned ch=0;ch<2;ch++){
   seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
   float x=i==0?.125f:i<128?((float)(seed&65535)-32768)/65536.f:0;
   emit(vc_boost_next(&s,x,ch));
  }
 }
}
