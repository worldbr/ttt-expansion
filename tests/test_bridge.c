#include <assert.h>
#include <stdio.h>
#include "battle_bridge.h"
int main(void){
    assert(ttt_bridge_validate(255,255,255,0,TTT_BRIDGE_FORM_LIMIT_OLD)==0);
    assert(ttt_bridge_validate(0,255,255,0,TTT_BRIDGE_FORM_LIMIT_OLD)==-11);
    assert(ttt_bridge_validate(255,71,255,0,TTT_BRIDGE_FORM_LIMIT_OLD)==-12);
    assert(ttt_bridge_validate(255,255,71,0,TTT_BRIDGE_FORM_LIMIT_OLD)==-13);
    assert(ttt_bridge_validate(255,255,255,255,TTT_BRIDGE_FORM_LIMIT_OLD)==-14);
    assert(ttt_bridge_validate(255,255,255,0,TTT_BRIDGE_FORM_LIMIT_NEW)==-15);
    assert(TTT_BRIDGE_A_ID00!=TTT_BRIDGE_B_ID00);
    assert(TTT_BRIDGE_VIDEL_FORM2+4==TTT_BRIDGE_VIDEL_END);
    puts("OK V0.9.1: validacao segura da ponte ID00");return 0;
}
