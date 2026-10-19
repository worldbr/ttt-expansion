#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "actor_inject.h"
int main(void){
    uint32_t out=0;
    assert(ttt_actor_inject_make_jal(TTT_ACTOR_STORE_PC,0x08B61234,TTT_ACTOR_STORE_EXPECTED,TTT_ACTOR_DELAY_EXPECTED,&out)==0);
    assert(out==(0x0C000000u|((0x08B61234u>>2)&0x03FFFFFFu)));
    assert(ttt_actor_inject_make_jal(TTT_ACTOR_STORE_PC,0x08B61234,0,TTT_ACTOR_DELAY_EXPECTED,&out)==-4);
    assert(ttt_actor_inject_make_jal(TTT_ACTOR_STORE_PC,0x08B61234,TTT_ACTOR_STORE_EXPECTED,0,&out)==-5);
    assert(ttt_actor_inject_make_jal(TTT_ACTOR_STORE_PC,0x18B61234,TTT_ACTOR_STORE_EXPECTED,TTT_ACTOR_DELAY_EXPECTED,&out)==-3);
    assert(ttt_actor_inject_make_jal(TTT_ACTOR_STORE_PC,0x08B61235,TTT_ACTOR_STORE_EXPECTED,TTT_ACTOR_DELAY_EXPECTED,&out)==-2);
    assert(ttt_actor_inject_enable(0,0)==-10); /* cannot arm until patched */
    printf("actor inject host validation OK\n");return 0;
}
