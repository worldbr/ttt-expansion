#include <assert.h>
#include <stdio.h>
#include "resource_hook.h"
int main(void){
    uint32_t word=0;
    assert(TTT_PACK_RESOURCE_EXTRA==0x3711u);
    assert(TTT_PACK_RESOURCE_EXTRA_RECORD==0x08B2A360u);
    assert(TTT_PACK_RESOURCE_GOKU_RECORD==0x08AF8B80u);
    assert(ttt_res_validate_pack(0x4B636150u,0x3712u,0x3711u,0x143000u)==0);
    assert(ttt_res_validate_pack(0x4B636150u,0x3734u,0x3711u,0x143000u)==0);
    assert(ttt_res_validate_pack(0x4B636150u,0x3711u,0x3711u,0x143000u)==-11);
    assert(ttt_res_validate_pack(0x4B636150u,0x3713u,0x3711u,0x143000u)==-11);
    assert(ttt_res_validate_pack(0,0x3712u,0x3711u,0x143000u)==-10);
    assert(ttt_res_validate_pack(0x4B636150u,0,0x3711u,0x143000u)==-11);
    assert(ttt_res_validate_pack(0x4B636150u,0x3712u,0x4A9u,0x143000u)==-12);
    assert(ttt_res_validate_pack(0x4B636150u,0x3712u,0x3711u,100)==-13);
    /* Unmodified alias still accepted for backward compatibility. */
    assert(ttt_res_validate_entry(0x2258Cu,0x2258Cu,0x143000u,0x143000u,0x3712u)==0);
    assert(ttt_res_validate_entry(0x2258Cu,0x2258Cu,0x143000u,0x143000u,0x3734u)==0);
    assert(ttt_res_validate_entry(0x2258Cu,0x2258Cu,0x12F220u,0x143000u,0x3734u)==-15);
    /* New actual physical record; RAM native offset must be later. */
    assert(ttt_res_validate_entry(0x80000u,0x2258Cu,0x12F220u,0x143000u,0x3734u)==0);
    assert(ttt_res_validate_entry(0x80000u,0x2258Cu,0x12F220u,0x143000u,0x3712u)==-16);
    assert(ttt_res_validate_entry(0x80000u,0x2258Cu,0x143000u,0x143000u,0x3734u)==-17);
    assert(ttt_res_validate_entry(0x10000u,0x2258Cu,0x12F220u,0x143000u,0x3734u)==-18);
    assert(ttt_res_validate_entry(0u,0x2258Cu,0x12F220u,0x143000u,0x3734u)==-14);
    assert(ttt_res_make_jal(0x0885520Cu,0x08B69000u,TTT_RES_CHECKED_JAL,TTT_RES_CHECKED_JAL,&word)==0);
    assert(word==(0x0C000000u|(0x08B69000u>>2)));
    assert(ttt_res_make_jal(0x0885520Cu,0x08B69000u,0,TTT_RES_CHECKED_JAL,&word)==-4);
    assert(ttt_res_make_jal(0x0885520Cu,0x18B69000u,TTT_RES_CHECKED_JAL,TTT_RES_CHECKED_JAL,&word)==-3);
    assert(ttt_res_should_redirect(1,0,0)==1);
    assert(ttt_res_should_redirect(0,0,0)==0);
    assert(ttt_res_should_redirect(1,3,0)==0);
    assert(ttt_res_should_redirect(1,0,1)==1);
    assert(ttt_res_should_redirect(1,1,2)==1);
    assert(ttt_res_should_redirect(1,2,3)==1);
    assert(ttt_res_should_redirect(1,2,4)==0);
    assert(ttt_res_index_for(0,0)==0x3711u);
    assert(ttt_res_index_for(0,3)==0x3714u);
    assert(ttt_res_index_for(1,0)==0x3716u);
    assert(ttt_res_index_for(1,3)==0x3719u);
    assert(ttt_res_index_for(2,0)==0x371Bu);
    assert(ttt_res_index_for(2,1)==0x371Cu);
    assert(ttt_res_index_for(2,2)==0x371Du);
    assert(ttt_res_index_for(2,3)==0x371Eu);
    assert(ttt_res_index_for(3,0)==0xffffffffu);
    puts("OK V0.9.8.6: indice por ID/color, alias legado e CHR fisico Early C1");
    return 0;
}
