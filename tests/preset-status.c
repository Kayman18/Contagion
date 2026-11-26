#include "presets.h"
#include <assert.h>
#include <stdio.h>
int main(void){unsigned mapped=0,partial=0,cache=0;assert(vc_preset_count()==650);
 for(unsigned i=0;i<650;i++){if(vc_preset_partial(i))partial++;else if(vc_preset_wave_cache(i))cache++;else mapped++;}
 assert(mapped==618&&partial==15&&cache==17);assert(!vc_preset_partial(650)&&!vc_preset_wave_cache(650));
 printf("{\"mapped\":%u,\"partial\":%u,\"cache\":%u,\"pass\":true}\n",mapped,partial,cache);
}
