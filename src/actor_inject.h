#ifndef TTT_ACTOR_INJECT_H
#define TTT_ACTOR_INJECT_H
#include <stdint.h>
/* ULUS10537 EBOOT.OLD. Exact instruction and delay slot validated
 * before patching. The hook only acts on explicitly enabled slots. */
#define TTT_ACTOR_STORE_PC UINT32_C(0x088576C8)
#define TTT_ACTOR_STORE_EXPECTED UINT32_C(0xAFC40000) /* sw a0,0(fp) */
#define TTT_ACTOR_DELAY_EXPECTED UINT32_C(0x8FA40018) /* lw a0,0x18(sp) */
#define TTT_ACTOR_SLOT_COUNT 4u
#define TTT_ACTOR_P1_ID_ADDRESS UINT32_C(0x08B48AE8)
#define TTT_ACTOR_P2_ID_ADDRESS UINT32_C(0x08B48BC0)
#define TTT_ACTOR_P3_ID_ADDRESS UINT32_C(0x08B48CB8)
#define TTT_ACTOR_P4_ID_ADDRESS UINT32_C(0x08B48D90)
#define TTT_ACTOR_P1_COLOR_ADDRESS UINT32_C(0x08B48AF0)

typedef struct {
    uint32_t installed,enabled,enabled_mask,error,original_insn,delay_insn,patch_insn;
    uint32_t hits,last_player,last_original_id,last_original_color,last_requested_id,last_requested_color;
    uint32_t slot_hits[TTT_ACTOR_SLOT_COUNT];
    uint32_t slot_original_id[TTT_ACTOR_SLOT_COUNT];
    uint32_t slot_original_color[TTT_ACTOR_SLOT_COUNT];
    uint32_t slot_requested_id[TTT_ACTOR_SLOT_COUNT];
    uint32_t slot_requested_color[TTT_ACTOR_SLOT_COUNT];
} TTTActorInjectStats;
/* Host-testable mirror of the MIPS shim's safety predicate. */
int ttt_actor_inject_slot_for_address(uint32_t actor_id_address);
int ttt_actor_inject_should_replace(uint32_t actor_id_address,uint32_t enabled_mask,uint32_t original_id);
int ttt_actor_inject_make_jal(uint32_t site,uint32_t target,uint32_t original,uint32_t delay,uint32_t *out);
int ttt_actor_inject_install(void);
/* player is zero-based, 0..3; only ID00 Color1 (index zero) supported. */
int ttt_actor_inject_enable_slot(int player,uint32_t external_id,uint32_t color);
void ttt_actor_inject_disable_slot(int player);
void ttt_actor_inject_disable(void);
/* Legacy 1P wrapper retained for native compatibility and upgrade ease. */
int ttt_actor_inject_enable(uint32_t external_id,uint32_t color);
void ttt_actor_inject_remove(void);
TTTActorInjectStats ttt_actor_inject_stats(void);
#endif
