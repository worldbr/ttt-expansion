#ifndef TTT_ACTOR_INJECT_H
#define TTT_ACTOR_INJECT_H
#include <stdint.h>
/* ULUS10537 EBOOT.OLD: instruction that writes the external actor ID.
 * Patch is OFF until the user explicitly confirms 1P Goku Early / Color 1.
 * All other actors execute the original store (recreated in the shim).
 */
#define TTT_ACTOR_STORE_PC UINT32_C(0x088576C8)
#define TTT_ACTOR_STORE_EXPECTED UINT32_C(0xAFC40000) /* sw a0,0(fp) */
#define TTT_ACTOR_DELAY_EXPECTED UINT32_C(0x8FA40018) /* lw a0,0x18(sp) */
#define TTT_ACTOR_P1_ID_ADDRESS UINT32_C(0x08B48AE8)
#define TTT_ACTOR_P1_COLOR_ADDRESS UINT32_C(0x08B48AF0)

typedef struct {
    uint32_t installed,enabled,error,original_insn,delay_insn,patch_insn;
    uint32_t hits,last_original_id,last_original_color,last_requested_id,last_requested_color;
} TTTActorInjectStats;
int ttt_actor_inject_make_jal(uint32_t site,uint32_t target,uint32_t original,uint32_t delay,uint32_t *out);
int ttt_actor_inject_install(void);
void ttt_actor_inject_disable(void);
int ttt_actor_inject_enable(uint32_t external_id,uint32_t color);
void ttt_actor_inject_remove(void);
TTTActorInjectStats ttt_actor_inject_stats(void);
#endif
