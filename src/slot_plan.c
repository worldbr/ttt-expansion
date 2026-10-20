#include "slot_plan.h"
void ttt_slot_plan_init(TTTSlotPlan *p){int i;
    if(!p)return;
    p->next_serial=0;
    for(i=0;i<4;++i){p->slot[i].confirmed=0;p->slot[i].external_id=0;
       p->slot[i].color_index=0;p->slot[i].resource_supported=0;p->slot[i].serial=0;}
}
int ttt_slot_plan_select(TTTSlotPlan *p,int player,int external_id,int color_index){
    TTTSlotChoice *s;
    if(!p||player<0||player>=4||external_id<0||external_id>0xA0||color_index<0||color_index>3)return -1;
    s=&p->slot[player];
    s->external_id=(uint8_t)external_id;
    s->color_index=(uint8_t)color_index;
    s->resource_supported=(uint8_t)(external_id<=2 && color_index<=3);
    s->confirmed=1;s->serial=++p->next_serial;
    return 0;
}
int ttt_slot_plan_get(const TTTSlotPlan *p,int player,TTTSlotChoice *out){
    if(!p||!out||player<0||player>=4)return -1;
    *out=p->slot[player];return 0;
}
void ttt_slot_plan_disable(TTTSlotPlan *p,int player){
    if(!p||player<0||player>=4)return;
    p->slot[player].confirmed=0;
    p->slot[player].resource_supported=0;
}
void ttt_slot_plan_reset(TTTSlotPlan *p){ttt_slot_plan_init(p);}
uint32_t ttt_slot_plan_mask(const TTTSlotPlan *p){int i;uint32_t bits=0;
    if(!p)return 0;
    for(i=0;i<4;i++)if(p->slot[i].confirmed)bits|=1u<<i;
    return bits;
}
