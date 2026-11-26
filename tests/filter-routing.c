#include "virooos.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static void emit(float x){assert(isfinite(x));fwrite(&x,4,1,stdout);}
int main(void){
    static const int balances[]={0,32,64,96,127};
    for(int sources=0;sources<4;sources++)for(int route=0;route<4;route++)for(int bi=0;bi<5;bi++)for(int mode=0;mode<4;mode++){
        VCSynth s;vc_init(&s,44100);VCVoice *v=&s.voice[0];
        s.param[VC_NOISE_VOLUME]=sources==3?80:0;vc_noise_setup(&v->noise,44100,20);v->noise.rng=0x12345678;
        s.param[VC_SUB_VOLUME]=sources?93:0;s.param[VC_SUB_SHAPE]=sources==2;s.param[VC_RING_VOLUME]=sources?79:0;v->inc[0]=1.f/64;
        s.param[VC_ROUTING]=route;s.param[VC_FILTER_BALANCE]=balances[bi];
        s.param[VC_PAN]=37;s.param[VC_SPREAD]=86;vc_route_setup(&s);
        /* Initialize per-voice cached source gains through normal note setup. */
        vc_note_on(&s,60,127);v->sub.phase=0;v->inc[0]=1.f/64;
        vc_noise_setup(&v->noise,44100,20);v->noise.rng=0x12345678;
        vc_filter_setup(&v->filter[0],44100,mode,route==1?4:2,73,55);
        vc_filter_setup(&v->filter[1],44100,(mode+1)%4,2,91,70);
        for(int j=0;j<1024;j++){
            float a=j==0?.75f:(j%31-15)/128.f,b=j==7?-.5f:(j%23-11)/128.f,l,r;
            float aa,bb;vc_mix_sources(&s,v,a,b,1,1,&aa,&bb);
            vc_route_sample(&s,v,aa,bb,&l,&r);emit(l);emit(r);
        }
    }
    for(unsigned slope=0;slope<128;slope++){
        VCEnvelope e={0};vc_env_setup(&e,44100,.005f,.001f,.4f,.015f);vc_env_slope(&e,44100,slope);e.stage=1;
        for(int j=0;j<6000;j++){if(j==4000)e.stage=4;emit(vc_env_next(&e));}
    }
    for(int variant=0;variant<4;variant++){
        VCSynth s;vc_init(&s,44100);
        vc_param(&s,VC_CUTOFF,47);vc_param(&s,VC_CUTOFF2,67);
        vc_param(&s,VC_F_ATTACK,12);vc_param(&s,VC_F_DECAY,55);vc_param(&s,VC_F_SUSTAIN,32);vc_param(&s,VC_F_RELEASE,44);
        vc_param(&s,VC_F_AMOUNT1,70);vc_param(&s,VC_F_AMOUNT2,81);
        vc_param(&s,VC_F_POLARITY1,variant&1);vc_param(&s,VC_F_POLARITY2,!(variant&1));
        vc_param(&s,VC_CUTOFF_LINK,variant>>1);
        vc_param(&s,VC_F_KEYFOLLOW1,90);vc_param(&s,VC_F_KEYFOLLOW2,33);vc_param(&s,VC_F_KEYBASE,48);
        int note=48+variant*8;vc_note_on(&s,note,100);
        for(int j=0;j<6000;j++){
            if(j==3000)vc_note_off(&s,note);
            float l,r;vc_render_stereo(&s,&l,&r,1);
            emit(s.voice[0].filter_env.level);
            for(int k=0;k<2;k++){float g=s.voice[0].filter[k].g;emit(g/(1+g));}
        }
    }
    return 0;
}
