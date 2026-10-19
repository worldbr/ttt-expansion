#include "actor_watch.h"
#include <string.h>
void ttt_actor_watch_init(TTTActorWatch *w){if(w)memset(w,0,sizeof(*w));}
static void record(TTTActorWatch *w,uint32_t frame,uint32_t i,uint32_t kind,
                   uint32_t old_id,uint32_t old_color,uint32_t id,uint32_t color,
                   uint32_t mask,int open){
    TTTActorChange *e;
    uint32_t seq=++w->events;
    e=&w->ring[(seq-1u)&(TTT_ACTOR_HISTORY-1u)];
    e->seq=seq;e->frame=frame;e->player=i+1u;e->kind=kind;
    e->previous_id=old_id;e->previous_color=old_color;
    e->id=id;e->color=color;e->plan_mask=mask;e->menu_open=(uint32_t)(open!=0);
}
int ttt_actor_watch_observe(TTTActorWatch *w,uint32_t frame,
             const uint32_t ids[TTT_ACTOR_COUNT],
             const uint32_t colors[TTT_ACTOR_COUNT],
             uint32_t plan_mask,int menu_open){
    uint32_t i;int changes=0;
    if(!w||!ids||!colors)return -1;
    for(i=0;i<TTT_ACTOR_COUNT;i++){
        uint32_t kind=0;
        if(!w->initialized)kind=TTT_ACTOR_INIT;
        else if(ids[i]!=w->last_id[i]||colors[i]!=w->last_color[i])
            kind=TTT_ACTOR_CHANGE;
        if(kind){
            record(w,frame,i,kind,w->last_id[i],w->last_color[i],
                   ids[i],colors[i],plan_mask,menu_open);
            changes++;
        }
        w->last_id[i]=ids[i];w->last_color[i]=colors[i];
    }
    w->initialized=1u;
    return changes;
}
int ttt_actor_watch_get(const TTTActorWatch *w,uint32_t seq,TTTActorChange *out){
    const TTTActorChange *e;
    if(!w||!out||!seq||seq>w->events||w->events-seq>=TTT_ACTOR_HISTORY)return -1;
    e=&w->ring[(seq-1u)&(TTT_ACTOR_HISTORY-1u)];
    if(e->seq!=seq)return -1;
    *out=*e;
    return 0;
}
