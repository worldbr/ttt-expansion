#ifndef TTT_BATTLE_BRIDGE_H
#define TTT_BATTLE_BRIDGE_H
#include <stdint.h>
/* ULUS10537 EBOOT.OLD: bridge to NEW FIFTH FORM of original Goku roster group.
 * Unlike V0.9.3, DO NOT modify Videl.  Goku original group has forms
 * [0,1,2,3,FF,0] at 0x08A73F60; the fifth form becomes 0x47 and a new
 * terminator 0xFF is written to the sixth entry (both validated first).
 * 0x47 = internal form index 71, shared experimentally by IDs 00..02.
 * This is a model-loading experiment, NOT three independently verified move sets.
 * Uses existing game selection UI. Direct match launch is NOT implemented.
 */
#define TTT_BRIDGE_A_ID00 UINT32_C(0x08A69720)
#define TTT_BRIDGE_B_ID00 UINT32_C(0x08A743E0)
#define TTT_BRIDGE_A_ID01 (TTT_BRIDGE_A_ID00+1u)
#define TTT_BRIDGE_A_ID02 (TTT_BRIDGE_A_ID00+2u)
#define TTT_BRIDGE_B_ID01 (TTT_BRIDGE_B_ID00+1u)
#define TTT_BRIDGE_B_ID02 (TTT_BRIDGE_B_ID00+2u)
#define TTT_BRIDGE_GOKU_GROUP UINT32_C(0x08A73F60)
/* Read-only control: Videl must stay exactly as in the original game. */
#define TTT_BRIDGE_VIDEL_FORM2 UINT32_C(0x08A740FC)
#define TTT_BRIDGE_VIDEL_END UINT32_C(0x08A74100)
#define TTT_BRIDGE_GOKU_FORM5 UINT32_C(0x08A73F70)
#define TTT_BRIDGE_GOKU_END UINT32_C(0x08A73F74)
#define TTT_BRIDGE_FORM_LIMIT UINT32_C(0x08A0662C)
#define TTT_BRIDGE_FORM_LIMIT_OLD UINT32_C(0x2CD10046)
#define TTT_BRIDGE_FORM_LIMIT_NEW UINT32_C(0x2CD10048)
#define TTT_BRIDGE_FORM_EXTRA UINT32_C(0x00000047)
#define TTT_BRIDGE_FORM_TERM UINT32_C(0x000000FF)

typedef struct TTTBridgeStats {
    int installed;
    int last_error;
    uint32_t a,b,goku_5,goku_end,limit,videl_2,videl_end;
    uint32_t installs,restores;
    uint32_t a1,b1,a2,b2;
} TTTBridgeStats;

/* Validates default group before patching, including original four forms. */
int ttt_bridge_validate(uint8_t a,uint8_t b,
                        const uint32_t goku_forms[6],uint32_t limit);
int ttt_bridge_install(void);
int ttt_bridge_remove(void);
TTTBridgeStats ttt_bridge_stats(void);
int ttt_bridge_is_active(void);
#endif
