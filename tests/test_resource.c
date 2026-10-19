#include <assert.h>
#include <stdio.h>
#include "resource_hook.h"
int main(void){
    uint32_t word=0;
    assert(TTT_PACK_RESOURCE_EXTRA==0x3711u);
    assert(TTT_PACK_RESOURCE_EXTRA_RECORD==0x08B2A360u);
    assert(TTT_PACK_RESOURCE_GOKU_RECORD==0x08AF8B80u);
    assert(ttt_res_validate_pack(0x4B636150u,0x3712u,0x3711u,0x143000u)==0);
    assert(ttt_res_validate_pack(0x4B636150u,0x3711u,0x3711u,0x143000u)==-11);
    assert(ttt_res_validate_pack(0x4B636150u,0x3713u,0x3711u,0x143000u)==-11);
    assert(ttt_res_validate_pack(0,0x3712u,0x3711u,0x143000u)==-10);
    assert(ttt_res_validate_pack(0x4B636150u,0,0x3711u,0x143000u)==-11);
    assert(ttt_res_validate_pack(0x4B636150u,0x3712u,0x4A9u,0x143000u)==-12);
    assert(ttt_res_validate_pack(0x4B636150u,0x3712u,0x3711u,100)==-13);
    assert(ttt_res_validate_alias(0x10ED6000u,0x10ED6000u,0x143000u,0x143000u)==0);
    assert(ttt_res_validate_alias(0x0u,0x10ED6000u,0x143000u,0x143000u)==-14);
    assert(ttt_res_validate_alias(0x10ED6800u,0x10ED6000u,0x143000u,0x143000u)==-14);
    assert(ttt_res_validate_alias(0x10ED6000u,0x10ED6000u,0x141680u,0x143000u)==-15);
    assert(ttt_res_make_jal(0x0885520Cu,0x08B69000u,TTT_RES_CHECKED_JAL,TTT_RES_CHECKED_JAL,&word)==0);
    assert(word==(0x0C000000u|(0x08B69000u>>2)));
    assert(ttt_res_make_jal(0x0885520Cu,0x08B69000u,0,TTT_RES_CHECKED_JAL,&word)==-4);
    assert(ttt_res_make_jal(0x0885520Cu,0x18B69000u,TTT_RES_CHECKED_JAL,TTT_RES_CHECKED_JAL,&word)==-3);
    assert(ttt_res_should_redirect(1,0,0)==1);
    assert(ttt_res_should_redirect(0,0,0)==0);
    assert(ttt_res_should_redirect(1,3,0)==0);
    assert(ttt_res_should_redirect(1,0,1)==0);
    puts("OK V0.9.8.2 resource JAL, new index 0x3711, PACKFILE 0x3712 alias");
    return 0;
}
