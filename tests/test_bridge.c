#include <assert.h>
#include <stdio.h>
#include "battle_bridge.h"
int main(void){
    const uint32_t good[6]={0,1,2,3,255,0};
    uint32_t bad[6]={0,1,2,3,255,0};
    assert(ttt_bridge_validate(255,255,good,TTT_BRIDGE_FORM_LIMIT_OLD)==0);
    assert(ttt_bridge_validate(0,255,good,TTT_BRIDGE_FORM_LIMIT_OLD)==-11);
    assert(ttt_bridge_validate(255,71,good,TTT_BRIDGE_FORM_LIMIT_OLD)==-12);
    assert(ttt_bridge_validate(255,255,good,TTT_BRIDGE_FORM_LIMIT_NEW)==-15);
    assert(ttt_bridge_validate(255,255,NULL,TTT_BRIDGE_FORM_LIMIT_OLD)==-10);
    bad[4]=71;assert(ttt_bridge_validate(255,255,bad,TTT_BRIDGE_FORM_LIMIT_OLD)==-13);bad[4]=255;
    bad[5]=255;assert(ttt_bridge_validate(255,255,bad,TTT_BRIDGE_FORM_LIMIT_OLD)==-14);bad[5]=0;
    for(int i=0;i<4;i++){
        bad[i]=75;
        assert(ttt_bridge_validate(255,255,bad,TTT_BRIDGE_FORM_LIMIT_OLD)==-16-i);
        bad[i]=(uint32_t)i;
    }
    assert(TTT_BRIDGE_GOKU_FORM5+4==TTT_BRIDGE_GOKU_END);
    assert(TTT_BRIDGE_GOKU_FORM5==TTT_BRIDGE_GOKU_GROUP+16);
    assert(TTT_BRIDGE_GOKU_GROUP!=UINT32_C(0x08A740F8)); /* Videl group untouched */
    assert(TTT_BRIDGE_VIDEL_FORM2==UINT32_C(0x08A740FC));
    assert(TTT_BRIDGE_VIDEL_END==UINT32_C(0x08A74100));
    puts("OK V0.9.4: Goku fifth form and safe original-data validation");return 0;
}
