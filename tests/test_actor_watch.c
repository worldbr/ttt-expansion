#include "actor_watch.h"
#include <assert.h>
#include <stdio.h>
int main(void){
    TTTActorWatch w;TTTActorChange e;
    uint32_t ids[4]={0x23,0x28,0x68,0x67},color[4]={0,0,0,0};
    ttt_actor_watch_init(&w);
    assert(ttt_actor_watch_observe(&w,1,ids,color,0,0)==4);
    assert(w.events==4);
    assert(ttt_actor_watch_get(&w,1,&e)==0 && e.kind==TTT_ACTOR_INIT && e.id==0x23);
    assert(ttt_actor_watch_observe(&w,2,ids,color,1,0)==0);
    ids[0]=0; color[0]=2;
    assert(ttt_actor_watch_observe(&w,3,ids,color,1,0)==1);
    assert(ttt_actor_watch_get(&w,5,&e)==0);
    assert(e.kind==TTT_ACTOR_CHANGE && e.player==1 && e.frame==3);
    assert(e.previous_id==0x23 && e.id==0 && e.color==2 && e.plan_mask==1);
    assert(ttt_actor_watch_get(&w,6,&e)==-1);
    for(uint32_t i=0;i<200;i++) {ids[1]++;ttt_actor_watch_observe(&w,4+i,ids,color,1,0);}
    assert(w.events==205);
    assert(ttt_actor_watch_get(&w,5,&e)==-1);
    assert(ttt_actor_watch_get(&w,205,&e)==0 && e.player==2);
    assert(ttt_actor_watch_observe(NULL,5,ids,color,1,1)==-1);
    puts("actor_watch PASS");return 0;
}
