#include "match_cycle.h"
void ttt_match_cycle_init(TTTMatchCycle *c){
    if(!c)return;
    c->hits_at_last_open=0;
    c->initialized=0;
}
int ttt_match_cycle_on_open(TTTMatchCycle *c,uint32_t hits,uint32_t mask){
    int completed;
    if(!c)return 0;
    completed=(c->initialized && hits!=c->hits_at_last_open && (mask&15u)!=0u);
    c->hits_at_last_open=hits;
    c->initialized=1;
    return completed;
}
