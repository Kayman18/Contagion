#include "matrix.h"
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv) {
    VCMatrix m; float sources[28],base[123],effective[123];
    if(argc>1 && !strcmp(argv[1],"factory")) {
        unsigned i=0; const VCMatrix *p;
        while((p=vc_matrix_factory(i++))) if(fwrite(p,sizeof(*p),1,stdout)!=1)return 2;
        return i==651?0:3;
    }
    if(argc>1 && !strcmp(argv[1],"cc")) {
        for(unsigned i=0;i<128;i++)printf("%d\n",vc_matrix_cc_source(i));
        return 0;
    }
    while(fread(&m,sizeof(m),1,stdin)==1) {
        if(fread(sources,sizeof(sources),1,stdin)!=1 || fread(base,sizeof(base),1,stdin)!=1)return 4;
        vc_matrix_apply(&m,sources,base,effective);
        if(argc>1 && !strcmp(argv[1],"filters")){float d[VC_MATRIX_FILTERS];vc_matrix_filters(&m,sources,d);for(unsigned i=0;i<VC_MATRIX_FILTERS;i++){const unsigned ids[VC_MATRIX_FILTERS]={24,25,26,27,28,29,30,31,76,77,78,79,48,49,54,55,32,3,56,60,81,16,18,53,75,6,11,52,72,73,7,12,47,74,9,14,17,45,46,1,2,80,59,58,33,34,35,36,37,38,39,40,41,42,43,44,50,51,57,5,97,98,99,91,15,10,19,21,23,90,20,22,4};unsigned dest=ids[i];effective[dest]=base[dest]+d[i];}}
        if(argc>1&&!strcmp(argv[1],"rounded")){effective[36]=vc_matrix_rounded(&m,sources,36,base[36]);effective[41]=vc_matrix_rounded(&m,sources,41,base[41]);}
        if(fwrite(effective,sizeof(effective),1,stdout)!=1)return 5;
    }
    return ferror(stdin)?6:0;
}
