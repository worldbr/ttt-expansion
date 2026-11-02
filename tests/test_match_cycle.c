#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "match_cycle.h"
int main(void){
    TTTMatchCycle c;
    ttt_match_cycle_init(&c);
    assert(ttt_match_cycle_on_open(&c,0,0)==0);
    /* Opening again before starting a match must not lose J1/J2 selections. */
    assert(ttt_match_cycle_on_open(&c,0,3)==0);
    /* One or more real actor-store injections since the previous opening. */
    assert(ttt_match_cycle_on_open(&c,2,3)==1);
    assert(ttt_match_cycle_on_open(&c,2,0)==0);
    assert(ttt_match_cycle_on_open(&c,2,1)==0);
    assert(ttt_match_cycle_on_open(&c,5,1)==1);
    /* No pending plans => no spurious reset. */
    assert(ttt_match_cycle_on_open(&c,6,0)==0);
    assert(ttt_match_cycle_on_open(&c,6,1)==0);
    puts("OK MENU47 V0.4.1: ciclo de batalha, confirmacoes nao ficam presas");
    return 0;
}
