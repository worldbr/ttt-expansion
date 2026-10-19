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
    assert(ttt_actor_inject_slot_for_address(0x08B48AE8u)==0);
    assert(ttt_actor_inject_slot_for_address(0x08B48BC0u)==1);
    assert(ttt_actor_inject_slot_for_address(0x08B48CB8u)==2);
    assert(ttt_actor_inject_slot_for_address(0x08B48D90u)==3);
    assert(ttt_actor_inject_slot_for_address(0x08B48AF0u)==-1); /* Color is not ID */
    assert(ttt_actor_inject_slot_for_address(0xDEADBEEFu)==-1);
    for(int i=0;i<4;i++){
        static const uint32_t ids[4]={0x08B48AE8u,0x08B48BC0u,0x08B48CB8u,0x08B48D90u};
        assert(ttt_actor_inject_should_replace(ids[i],1u<<i,0x19u));
        assert(!ttt_actor_inject_should_replace(ids[i],0,0x19u));
        assert(!ttt_actor_inject_should_replace(ids[i],1u<<((i+1)%4),0x19u));
        assert(!ttt_actor_inject_should_replace(ids[i],1u<<i,0u)); /* empty */
        assert(!ttt_actor_inject_should_replace(ids[i],1u<<i,0xFFu)); /* invalid */
        assert(!ttt_actor_inject_should_replace(ids[i],1u<<i,0xFFFFFFFFu));
        assert(ttt_actor_inject_should_replace(ids[i],0xFu,0xA0u));
    }
    assert(!ttt_actor_inject_should_replace(0x08B48AF0u,0xFu,0x19u));
    assert(ttt_actor_inject_enable_slot(4,0,0)==-12);
    assert(ttt_actor_inject_enable_slot(-1,0,0)==-12);
    assert(ttt_actor_inject_enable_slot(2,0,0)==-10);
    assert(ttt_actor_inject_enable(0,0)==-10); /* cannot arm until patched */
    printf("actor inject 1P-4P host validation OK\n");return 0;
}
