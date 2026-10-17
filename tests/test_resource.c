#include <assert.h>
#include <stdio.h>
#include "resource_hook.h"
int main(void){
    uint32_t word=0;
    assert(TTT_PACK_RESOURCE_EXTRA==0x4A9u);
    assert(TTT_PACK_RESOURCE_EXTRA_RECORD==0x08AF7CE0u);
    assert(TTT_PACK_RESOURCE_GOKU_RECORD==0x08AF8B80u);
    assert(ttt_res_validate_pack(0x4B636150u,0x3711u,0x4A9u,0x143000u)==0);
    assert(ttt_res_validate_pack(0,0x3711u,0x4A9u,0x143000u)==-10);
    assert(ttt_res_validate_pack(0x4B636150u,0,0x4A9u,0x143000u)==-11);
    assert(ttt_res_validate_pack(0x4B636150u,0x3711u,0x4AAu,0x143000u)==-12);
    assert(ttt_res_validate_pack(0x4B636150u,0x3711u,0x4A9u,100)==-13);
    assert(ttt_res_make_jal(0x0885520Cu,0x08B69000u,TTT_RES_CHECKED_JAL,TTT_RES_CHECKED_JAL,&word)==0);
    assert(word==(0x0C000000u|(0x08B69000u>>2)));
    assert(ttt_res_make_jal(0x0885520Cu,0x08B69000u,0,TTT_RES_CHECKED_JAL,&word)==-4);
    assert(ttt_res_make_jal(0x0885520Cu,0x18B69000u,TTT_RES_CHECKED_JAL,TTT_RES_CHECKED_JAL,&word)==-3);
    assert(ttt_res_should_redirect(1,0,0)==1);
    assert(ttt_res_should_redirect(0,0,0)==0);
    assert(ttt_res_should_redirect(1,3,0)==0);
    assert(ttt_res_should_redirect(1,0,1)==0);
    puts("OK V0.9.3 resource JAL plan and PACKFILE validation");
    return 0;
}
