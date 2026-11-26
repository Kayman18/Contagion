#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void emit(float x){assert(__builtin_isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
 const float notes[]={-36,0,36};
 for(unsigned curve=0;curve<=14;curve++)for(float level=64.375f;level<127;level+=1)for(unsigned note=0;note<3;note++){
  VCSaturation s={0};vc_sat_setup(&s,44100,curve,level,notes[note]);
  if(curve==9||curve==10){emit(s.period);s.period=floorf(s.period*16+.5f)/16;}
  else if(curve>=11)emit(s.coef);
  for(unsigned i=0;i<256;i++){float x=(i%67-33.f)/16;emit(vc_sat_next(&s,x));}
 }
 for(unsigned route=0;route<4;route++)for(unsigned curve=0;curve<=14;curve++){
  VCSynth s;vc_init(&s,44100);s.param[VC_ROUTING]=route;s.param[VC_SAT_CURVE]=curve;s.param[VC_SAT_DRIVE]=96;s.param[VC_CUTOFF]=73;s.param[VC_CUTOFF2]=91;s.param[VC_RESONANCE]=55;s.param[VC_RESONANCE2]=70;vc_route_setup(&s);vc_note_on(&s,60,127);VCVoice *v=&s.voice[0];vc_filter_control(&s,v);
  if(curve==9||curve==10)v->saturation.period=floorf(v->saturation.period*16+.5f)/16;
  for(unsigned i=0;i<1024;i++){float l,r;vc_route_sample(&s,v,(i%31-15.f)/16,(i%23-11.f)/16,&l,&r);emit(l);emit(r);}
 }
 /* Mode/drive changes preserve filter and reducer state, as the source does. */
 VCSaturation s={0};vc_sat_setup(&s,44100,11,96,0);for(unsigned i=0;i<100;i++)vc_sat_next(&s,.5f);float lp=s.lp;
 vc_sat_setup(&s,44100,0,64,0);assert(vc_sat_next(&s,-.25f)==-.25f&&s.lp==lp);vc_sat_setup(&s,44100,13,100,12);assert(s.lp==lp);
}
