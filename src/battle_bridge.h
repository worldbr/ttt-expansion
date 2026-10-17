#ifndef TTT_BATTLE_BRIDGE_H
#define TTT_BATTLE_BRIDGE_H
#include <stdint.h>
/* ULUS10537 EBOOT.OLD: experimental carrier using Videl form slot #2.
   NOT an external loader. It aliases Goku Base CHR/SFX for external ID 00.
   Installation requires unmodified original bytes at all sites. */
#define TTT_BRIDGE_A_ID00 UINT32_C(0x08A69720)
#define TTT_BRIDGE_B_ID00 UINT32_C(0x08A743E0)
#define TTT_BRIDGE_VIDEL_FORM2 UINT32_C(0x08A740FC)
#define TTT_BRIDGE_VIDEL_END UINT32_C(0x08A74100)
#define TTT_BRIDGE_FORM_LIMIT UINT32_C(0x08A0662C)
#define TTT_BRIDGE_FORM_LIMIT_OLD UINT32_C(0x2CD10046)
#define TTT_BRIDGE_FORM_LIMIT_NEW UINT32_C(0x2CD10048)

typedef struct TTTBridgeStats {
    int installed;
    int last_error;
    uint32_t a, b, videl_2, videl_end, limit;
    uint32_t installs, restores;
} TTTBridgeStats;

/* Portable validator, for native tests and for checking the actual game memory. */
int ttt_bridge_validate(uint8_t a,uint8_t b,uint32_t videl2,uint32_t end,uint32_t limit);
int ttt_bridge_install(void);
int ttt_bridge_remove(void);
TTTBridgeStats ttt_bridge_stats(void);
int ttt_bridge_is_active(void);
#endif
