#include <assert.h>
#include <stdio.h>
#include "slot_plan.h"
int main(void){
    TTTSlotPlan p;TTTSlotChoice a,b;
    ttt_slot_plan_init(&p);
    assert(ttt_slot_plan_mask(&p)==0);
    assert(ttt_slot_plan_select(&p,0,0,0)==0);
    assert(ttt_slot_plan_select(&p,1,1,2)==0);
    assert(ttt_slot_plan_select(&p,2,0,3)==0);
    assert(ttt_slot_plan_select(&p,3,0,0)==0);
    assert(ttt_slot_plan_mask(&p)==15u);
    assert(ttt_slot_plan_get(&p,0,&a)==0 && a.external_id==0 && a.resource_supported==1);
    assert(ttt_slot_plan_get(&p,1,&b)==0 && b.external_id==1 && b.color_index==2 && b.resource_supported);
    assert(ttt_slot_plan_get(&p,3,&b)==0 && b.resource_supported==1 && b.serial==4);
    assert(ttt_slot_plan_select(&p,3,1,4)==-1);
    assert(ttt_slot_plan_select(&p,4,1,0)==-1);
    assert(ttt_slot_plan_select(&p,0,161,0)==-1);
    assert(ttt_slot_plan_select(&p,0,0,0)==0 && p.slot[0].serial==5);
    ttt_slot_plan_disable(&p,1);
    assert(ttt_slot_plan_mask(&p)==13u);
    ttt_slot_plan_disable(&p,-1);
    assert(ttt_slot_plan_mask(&p)==13u);
    ttt_slot_plan_reset(&p);assert(ttt_slot_plan_mask(&p)==0);
    puts("OK V0.9.8.6: 1P-4P, 3 IDs e 4 colors no plano");return 0;
}
