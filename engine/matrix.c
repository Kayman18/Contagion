#include "matrix.h"
#include "matrix.inc"
/* Amount destinations in original execution order: assign1, assign2 x2,
 * assign3 x3, assign4/5/6, then the two dedicated LFO assignments. */
static const uint8_t amount_destination[VC_MATRIX_ROUTES] =
    {82,83,84,85,86,87,120,121,122,100,101};
static int amount_target(unsigned d) {
    return (d>=82 && d<=87) || d==100 || d==101 || (d>=120 && d<=122);
}
void vc_matrix_apply(const VCMatrix *m,const float *sources,
                     const float *base,float *effective) {
    unsigned i,pass;
    for(i=0;i<VC_MATRIX_DESTINATIONS;i++) effective[i]=base[i];
    /* Route amounts are part of the effective patch, so earlier routes can
     * affect later routes in the first pass. Do not precompute all amounts. */
    for(i=0;i<VC_MATRIX_ROUTES;i++) effective[amount_destination[i]]=m->route[i].amount;
    for(pass=0;pass<2;pass++) for(i=0;i<VC_MATRIX_ROUTES;i++) {
        const VCMatrixRoute *r=&m->route[i];
        unsigned d=r->destination;
        if(!r->source || r->source>=VC_MATRIX_SOURCES ||
           d>=VC_MATRIX_DESTINATIONS || !matrix_destination_active[d] ||
           amount_target(d)!=(pass==0)) continue;
        float a=effective[amount_destination[i]];
        if(a<0) a=0;
        if(a>127) a=127;
        if(a!=64) effective[d]+=2*(a-64)*sources[r->source];
    }
}
int vc_matrix_cc_source(unsigned cc) {
    switch(cc) {
    case 1:return 3; case 2:return 4; case 3:return 5; case 4:return 6;
    case 6:return 7; case 8:return 8; case 9:return 9; case 11:return 10;
    case 12:return 11; case 13:return 12; case 14:return 13; case 15:return 14;
    case 16:return 15; case 64:return 16; case 65:return 17; case 66:return 18;
    default:return -1;
    }
}

/* Sound destinations cannot affect amount evaluation in the original second
 * pass. Keep only eleven amounts and the connected outputs for filter-only playback. */
static int amount_index(unsigned d) {
 if(d>=82 && d<=87)return (int)d-82;
 if(d>=120 && d<=122)return (int)d-114;
 if(d==100 || d==101)return (int)d-91;
 return -1;
}
static int filter_index(unsigned d){
 if(d>=24 && d<=31)return (int)d-24;
 if(d>=76 && d<=79)return (int)d-68;
 if(d==48 || d==49)return (int)d-36;
 if(d==54 || d==55)return (int)d-40;
 if(d==32)return 16;
 if(d==3)return 17;
 if(d==56)return 18;
 if(d==60)return 19;
 if(d==81)return 20;
 if(d==16)return 21;
 if(d==18)return 22;
 if(d==53)return 23;
 if(d==75)return 24;
 if(d==6)return 25;
 if(d==11)return 26;
 if(d==52)return 27;
 if(d==72)return 28;
 if(d==73)return 29;
 if(d==7)return 30;
 if(d==12)return 31;
 if(d==47)return 32;
 if(d==74)return 33;
 if(d==9)return 34;
 if(d==14)return 35;
 if(d==17)return 36;
 if(d==45)return 37;
 if(d==46)return 38;
 if(d==1)return 39;
 if(d==2)return 40;
 if(d==80)return 41;
 if(d==59)return 42;
 if(d==58)return 43;
 if(d>=33&&d<=42)return 44+(int)d-33;
 if(d==43)return 54;
 if(d==44)return 55;
 if(d==50)return 56;
 if(d==51)return 57;
 if(d==57)return 58;
 if(d==5)return 59;
 if(d>=97&&d<=99)return 60+(int)d-97;
 if(d==91)return 63;
 if(d==15)return 64;
 if(d==10)return 65;
 if(d==19)return 66;
 if(d==21)return 67;
 if(d==23)return 68;
 if(d==90)return 69;
 if(d==20)return 70;
 if(d==22)return 71;
 if(d==4)return 72;
 return -1;
}
int vc_matrix_has_filters(const VCMatrix *m) {
 if(!m)return 0;
 for(unsigned i=0;i<11;i++)if(m->route[i].source>0 && m->route[i].source<28 &&
  (filter_index(m->route[i].destination)>=0
#ifdef VC_BAND_CACHE
   ||m->route[i].destination==8||m->route[i].destination==13
#endif
  ))return 1;
 return 0;
}
void vc_matrix_filters(const VCMatrix *m,const float *sources,float *delta) {
 float amount[11];
 for(unsigned i=0;i<11;i++)amount[i]=m->route[i].amount;
 for(unsigned i=0;i<VC_MATRIX_FILTERS;i++)delta[i]=0;
 for(unsigned pass=0;pass<2;pass++)for(unsigned i=0;i<11;i++){
  const VCMatrixRoute *r=&m->route[i];
  if(!r->source || r->source>=28)continue;
  int d=pass ? filter_index(r->destination) : amount_index(r->destination);
  if(d<0)continue;
  float a=amount[i];if(a<0)a=0;if(a>127)a=127;
  if(a!=64){float v=2*(a-64)*sources[r->source];if(pass)delta[d]+=v;else amount[d]+=v;}
 }
}

/* Float pairs retain the original source's extra precision only at discrete
 * half-step boundaries. Avoid software doubles in the target control path. */
typedef struct {float hi,lo;} MatrixPair;
static MatrixPair pair_add(MatrixPair a,MatrixPair b){
 float sum=a.hi+b.hi,z=sum-a.hi,error=(a.hi-(sum-z))+(b.hi-z)+a.lo+b.lo;
 float hi=sum+error;return (MatrixPair){hi,error-(hi-sum)};
}
static MatrixPair pair_scale(MatrixPair a,float b){
 float p=a.hi*b,ca=4097*a.hi,cb=4097*b,ah=ca-(ca-a.hi),bh=cb-(cb-b);
 float al=a.hi-ah,bl=b-bh,error=((ah*bh-p)+ah*bl+al*bh)+al*bl+a.lo*b;
 float hi=p+error;return (MatrixPair){hi,error-(hi-p)};
}
static MatrixPair pair_amount(MatrixPair a){
 if(a.hi<0||(a.hi==0&&a.lo<0))return (MatrixPair){0,0};
 if(a.hi>127||(a.hi==127&&a.lo>0))return (MatrixPair){127,0};
 return a;
}
unsigned vc_matrix_rounded(const VCMatrix *m,const float *sources,unsigned destination,float base){
 MatrixPair amount[11],out={base,0};
 for(unsigned i=0;i<11;i++)amount[i]=(MatrixPair){m->route[i].amount,0};
 for(unsigned pass=0;pass<2;pass++)for(unsigned i=0;i<11;i++){
  const VCMatrixRoute *r=&m->route[i];if(!r->source||r->source>=28)continue;
  int d=pass?(r->destination==destination?0:-1):amount_index(r->destination);if(d<0)continue;
  MatrixPair a=pair_add(pair_amount(amount[i]),(MatrixPair){-64,0});
  MatrixPair value=pair_scale(a,2*sources[r->source]);
  if(pass)out=pair_add(out,value);else amount[d]=pair_add(amount[d],value);
 }
 out=pair_amount(out);unsigned i=(unsigned)out.hi;float f=out.hi-i;
 return i+(f>.5f||(f==.5f&&out.lo>=0));
}
