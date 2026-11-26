#include "../engine/virooos.c"
#include <assert.h>
#include <stdio.h>
static void near(float a,float b){assert(fabsf(a-b)<.0001f);}
int main(void){
 for(unsigned count=1;count<=4;count++){
  VCSynth s;vc_init(&s,44100);vc_param(&s,VC_UNISON_MODE,count-1);
  vc_param(&s,VC_LFO_TRIGGER,33);vc_param(&s,VC_UNISON_PHASE,127);
  vc_note_on(&s,60,100);assert(vc_active(&s)==count);
  for(unsigned d=0;d<128;d++){
   vc_param(&s,VC_UNISON_DETUNE,d);
   for(unsigned i=0;i<count;i++){
    VCVoice *v=&s.voice[i];float side=count>1?2.f*i/(count-1)-1:0;
    near(v->unison_side,side);
    near(v->unison_cents,side==0?0:side*vc_unison_detune[d]+((count&1)?2.37f:0));
    if(!d){float ph=.25f+(count>1?.5f*i/(count-1):0);near(v->lfo.phase,ph);}
    for(unsigned w=0;w<128;w++){
     vc_param(&s,VC_SPREAD,w);vc_lfo2_pan_control(&s,v);
     float base[4];vc_pan_pair(s.param[VC_PAN],base);vc_pan_pair(s.param[VC_PAN],base+2);
     float width=w>64?64:w,a=1+fabsf(side)*(.751f+.00686f*width-1),b=1+fabsf(side)*(.751f*(1-width/64)-1);
     if(side)for(unsigned j=0;j<4;j++)near(v->pan_target[j],base[j]*(((j&1)?side>=0:side<=0)?a:b));
    }
   }
  }
  float l[512],r[512];vc_render_stereo(&s,l,r,512);float energy=0;
  for(unsigned j=0;j<512;j++){assert(__builtin_isfinite(l[j])&&__builtin_isfinite(r[j]));energy+=fabsf(l[j])+fabsf(r[j]);}assert(energy>0);
  vc_cc(&s,64,127);vc_note_off(&s,60);for(unsigned i=0;i<count;i++)assert(s.voice[i].env.stage!=4);
  vc_cc(&s,64,0);for(unsigned i=0;i<count;i++)assert(!s.voice[i].held&&s.voice[i].env.stage==4);
  vc_panic(&s);vc_note_on(&s,60,100);vc_note_on(&s,64,100);vc_note_on(&s,67,100);
  assert(vc_active(&s)<=4);
  if(count==2){unsigned old=0,newest=0;for(unsigned i=0;i<4;i++){old+=s.voice[i].note==60;newest+=s.voice[i].note==67;}assert(!old&&newest==2);}
  if(count>=3)for(unsigned i=0;i<4;i++)if(s.voice[i].env.stage)assert(s.voice[i].note==67);
  VCSynth a,b;vc_init(&a,44100);vc_param(&a,VC_UNISON_MODE,count-1);vc_note_on(&a,60,100);b=a;
  float ll[512],rr[512];vc_render_stereo(&a,l,r,512);
  for(unsigned j=0;j<512;j++)vc_render_stereo(&b,ll+j,rr+j,1);
  assert(!memcmp(l,ll,sizeof l)&&!memcmp(r,rr,sizeof r));
  vc_init(&a,44100);vc_param(&a,VC_UNISON_MODE,count-1);vc_param(&a,VC_KEY_MODE,5);
  vc_note_on(&a,60,100);vc_note_off(&a,60);assert(vc_active(&a)==count);
  vc_note_on(&a,67,100);
  for(unsigned i=0;i<4;i++)if(a.voice[i].held)assert(a.voice[i].note==67);
  vc_param(&s,VC_UNISON_MODE,count==4?0:count);assert(!vc_active(&s));
  for(unsigned mode=1;mode<=4;mode++){
   vc_init(&s,44100);vc_param(&s,VC_UNISON_MODE,count-1);vc_param(&s,VC_KEY_MODE,mode);
   vc_note_on(&s,60,100);vc_note_on(&s,67,90);assert(vc_active(&s)==count);
   for(unsigned i=0;i<count;i++)assert(s.voice[i].note==67);
   vc_note_off(&s,67);for(unsigned i=0;i<count;i++)assert(s.voice[i].note==60&&s.voice[i].held);
   vc_note_off(&s,60);for(unsigned i=0;i<count;i++)assert(!s.voice[i].held&&s.voice[i].env.stage==4);
  }
 }
 puts("PASS: unison detune, stereo width, LFO phase, audio, sustain, group stealing, mono priority and mode transitions");
}
