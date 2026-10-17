#include <assert.h>
#include <stdint.h>
#include "../src/external_probe.h"
int main(void){
    const unsigned char x[]={0x01,0x02,0x03};
    assert(ttt_probe_validate_size(0x143000,0x143000)==0);
    assert(ttt_probe_validate_size(0x143000,0x141680)==0); /* user file */
    assert(ttt_probe_validate_size(0x143000,0x13FD40)==0); /* extracted CHR */
    assert(ttt_probe_validate_size(0x143000,0x142000)==0);
    assert(ttt_probe_validate_size(0x143000,0)<0);
    assert(ttt_probe_validate_size(0x143000,0x100)<0);
    assert(ttt_probe_validate_size(0x143000,0x02000001)<0);
    assert(ttt_probe_validate_size(0x02000001,0x141680)<0);
    assert(ttt_probe_hash_update(2166136261u,x,3)==UINT32_C(0x56CF37AB));
    return 0;
}
