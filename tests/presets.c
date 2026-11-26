#include "virooos.h"
#include "presets.h"
#include <assert.h>
#include <stdio.h>
int main(void){
 assert(vc_preset_count()==650);assert(!vc_preset_params(650));assert(vc_preset_find(0,16)==-1);
 for(unsigned i=0;i<vc_preset_count();i++){
  const uint8_t *p=vc_preset_params(i);VCSynth s;vc_init(&s,44100);vc_apply_patch(&s,p);
  assert(vc_preset_find(vc_preset_bank(i),vc_preset_program(i))==(int)i);
  assert(vc_preset_name(i)[0]);for(unsigned id=0;id<VC_PARAMS;id++)assert(s.param[id]==(p[id]>vc_param_max(id)?vc_param_max(id):p[id]));
  unsigned char bank=(unsigned char)vc_preset_bank(i),program=(unsigned char)vc_preset_program(i);fwrite(&bank,1,1,stdout);fwrite(&program,1,1,stdout);fwrite(vc_preset_name(i),1,11,stdout);fwrite(s.param,1,VC_PARAMS,stdout);
 }
}
