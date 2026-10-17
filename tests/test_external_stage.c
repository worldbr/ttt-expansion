#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../src/external_stage.h"
#include "../src/external_probe.h"

int main(void) {
    FILE *f;
    TTTExternalStage a,b;
    uint32_t expect=UINT32_C(2166136261);
    const char *tmp="/tmp/ttt_stage_smoke.pck1";
    const uint32_t n=UINT32_C(0x141680);
    unsigned char x[4096];
    uint32_t i,j;
    f=fopen(tmp,"wb");assert(f);
    for(i=0;i<n;i+=(uint32_t)sizeof(x)){
        uint32_t take=(n-i)>sizeof(x)?sizeof(x):(n-i);
        for(j=0;j<take;j++)x[j]=(unsigned char)(((i+j)*13u+17u)&255u);
        assert(fwrite(x,1,take,f)==take);
        expect=ttt_probe_hash_update(expect,x,take);
    }
    fclose(f);
    assert(ttt_external_stage_load(tmp,0x143000,&a)==0);
    assert(a.result==0 && a.retained==1 && a.address!=0);
    assert(a.actual_size==n && a.bytes_read==n && a.fnv1a==expect);
    assert(ttt_external_stage_size()==n);
    assert(ttt_external_stage_data()!=NULL);
    assert(ttt_external_stage_data()[0]==17);
    assert(ttt_external_stage_load("/tmp/nonexistent_ttt_stage_file.zzz",0,&b)==-1);
    assert(b.retained==0 && ttt_external_stage_size()==0 && !ttt_external_stage_data());
    ttt_external_stage_release();
    remove(tmp);
    return 0;
}
