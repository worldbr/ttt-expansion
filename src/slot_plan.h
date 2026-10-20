#ifndef TTT_SLOT_PLAN_H
#define TTT_SLOT_PLAN_H
#include <stdint.h>
#define TTT_SLOT_COUNT 4u
/* Explicit 1P/2P/3P/4P desired roster. READ-ONLY plans: do not mutate
 * combat actor memory until a verified pre-load player-slot hook exists. */
typedef struct {
    uint8_t confirmed, external_id, color_index, resource_supported;
    uint32_t serial;
} TTTSlotChoice;
typedef struct {
    TTTSlotChoice slot[TTT_SLOT_COUNT];
    uint32_t next_serial;
} TTTSlotPlan;
void ttt_slot_plan_init(TTTSlotPlan *p);
int ttt_slot_plan_select(TTTSlotPlan *p,int player,int external_id,int color_index);
int ttt_slot_plan_get(const TTTSlotPlan *p,int player,TTTSlotChoice *out);
void ttt_slot_plan_disable(TTTSlotPlan *p,int player);
void ttt_slot_plan_reset(TTTSlotPlan *p);
uint32_t ttt_slot_plan_mask(const TTTSlotPlan *p);
#endif
