#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "callsite_plan.h"
int main(void) {
    const uint32_t sites[3]={UINT32_C(0x08A63A48),UINT32_C(0x08A63B10),UINT32_C(0x08A6408C)};
    uint32_t patched=0;
    for(unsigned i=0;i<3;i++) {
        assert(ttt_callsite_plan(sites[i],0x09FC1234u,0x0E299AA1u,&patched)==0);
        assert(patched==(0x0C000000u|(0x09FC1234u>>2)));
        assert(ttt_callsite_plan(sites[i],0x09FC1234u,0x00000000u,&patched)==-12);
        assert(ttt_callsite_plan(sites[i],0x09FC1235u,0x0E299AA1u,&patched)==-11);
        assert(ttt_callsite_plan(sites[i],0x18FC1234u,0x0E299AA1u,&patched)==-10);
    }
    printf("OK: 3 JALs; validacao e recusa de opcodes inesperados\n");
    return 0;
}
