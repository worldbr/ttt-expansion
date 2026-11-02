#ifndef TTT_MATCH_CYCLE_H
#define TTT_MATCH_CYCLE_H
#include <stdint.h>
/* Tracks the completed actor-injection phase across overlay openings.
 * A successful actor-store write is our strongest existing evidence that a
 * match was prepared. Reopening the overlay afterwards starts a fresh round.
 * It does NOT claim to detect the game's native READY flag or scene. */
typedef struct TTTMatchCycle {
    uint32_t hits_at_last_open;
    uint8_t initialized;
} TTTMatchCycle;
void ttt_match_cycle_init(TTTMatchCycle *c);
/* Returns 1 only when a previously armed set of player choices should be
 * reset at this opening.  Does not reset just because the user reopens the
 * plugin several times while preparing a single match. */
int ttt_match_cycle_on_open(TTTMatchCycle *c,uint32_t total_actor_hits,
                            uint32_t planned_player_mask);
#endif
