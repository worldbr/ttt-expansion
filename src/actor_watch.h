#ifndef TTT_ACTOR_WATCH_H
#define TTT_ACTOR_WATCH_H
#include <stdint.h>

/* Read-only chronology of combat actor fields (NOT the selection structure).
 * A record means the values changed between VBlank samples. It DOES NOT
 * prove which instruction wrote them. The PPSSPP write breakpoint does that. */
#define TTT_ACTOR_COUNT 4u
#define TTT_ACTOR_HISTORY 128u
#define TTT_ACTOR_INIT 1u
#define TTT_ACTOR_CHANGE 2u
typedef struct {
    uint32_t seq,frame,player,kind;
    uint32_t previous_id,previous_color,id,color;
    uint32_t plan_mask,menu_open;
} TTTActorChange;
typedef struct {
    uint32_t last_id[TTT_ACTOR_COUNT],last_color[TTT_ACTOR_COUNT];
    uint32_t events,initialized;
    TTTActorChange ring[TTT_ACTOR_HISTORY];
} TTTActorWatch;
void ttt_actor_watch_init(TTTActorWatch *w);
/* Return number of new events. Pure: no memory-mapped reads or I/O. */
int ttt_actor_watch_observe(TTTActorWatch *w,uint32_t frame,
             const uint32_t ids[TTT_ACTOR_COUNT],
             const uint32_t colors[TTT_ACTOR_COUNT],
             uint32_t plan_mask,int menu_open);
/* Return 0 when record still available, -1 for out-of-range/overwritten. */
int ttt_actor_watch_get(const TTTActorWatch *w,uint32_t seq,TTTActorChange *out);
#endif
