#ifndef TTT_RESOURCE_HOOK_H
#define TTT_RESOURCE_HOOK_H
#include <stdint.h>
/* ULUS10537 only: resource 1194 is zero-based PACKFILE entry 0x4A9.
 * Does NOT load external files; the game's original PACKFILE loader is used. */
#define TTT_PACK_HEADER UINT32_C(0x08AF3240)
#define TTT_PACK_RECORD_BASE UINT32_C(0x08AF3250)
#define TTT_PACK_RESOURCE_GOKU UINT32_C(0x593)
#define TTT_PACK_RESOURCE_EXTRA UINT32_C(0x4A9)
#define TTT_PACK_RESOURCE_EXTRA_RECORD (TTT_PACK_RECORD_BASE + 16u * TTT_PACK_RESOURCE_EXTRA)
#define TTT_PACK_RESOURCE_GOKU_RECORD (TTT_PACK_RECORD_BASE + 16u * TTT_PACK_RESOURCE_GOKU)
#define TTT_RES_CHECKED_JAL UINT32_C(0x0E271B28)
#define TTT_RES_UNCHECKED_JAL UINT32_C(0x0E271B58)

typedef struct {
    int installed, last_error;
    uint32_t calls_checked,calls_unchecked,redirects;
    uint32_t last_id,last_variant,last_original,last_result;
    uint32_t pack_magic,pack_count,resource_id,resource_size;
    uint32_t original_size;
} TTTResourceHookStats;

/* Pure validation for host-side testing. No changes to game memory. */
int ttt_res_make_jal(uint32_t site,uint32_t dest,uint32_t observed,
                      uint32_t expected,uint32_t *patched);
int ttt_res_validate_pack(uint32_t magic,uint32_t count,uint32_t resource_id,
                          uint32_t size);
int ttt_res_hook_install(void);
void ttt_res_hook_remove(void);
TTTResourceHookStats ttt_res_hook_stats(void);
#endif
