#ifndef TTT_RESOURCE_HOOK_H
#define TTT_RESOURCE_HOOK_H
#include <stdint.h>
/* ULUS10537 only: experimental PACKFILE has 0x3712 entries.
 * Last zero-based entry 0x3711 (displayed as file 14098) is an encoded
 * ALIAS of the ORIGINAL Goku Base CHR (index 0x593), no file appended.
 * Does NOT load external data; game's PACKFILE loader remains in control.
 * STRICT initial validation: count==0x3712 AND alias matches Goku Base. */
#define TTT_PACK_HEADER UINT32_C(0x08AF3240)
#define TTT_PACK_RECORD_BASE UINT32_C(0x08AF3250)
#define TTT_PACK_RESOURCE_GOKU UINT32_C(0x593)
#define TTT_PACK_RESOURCE_EXTRA UINT32_C(0x3711)
#define TTT_PACK_RESOURCE_EXTRA_RECORD (TTT_PACK_RECORD_BASE + 16u * TTT_PACK_RESOURCE_EXTRA)
#define TTT_PACK_RESOURCE_GOKU_RECORD (TTT_PACK_RECORD_BASE + 16u * TTT_PACK_RESOURCE_GOKU)
#define TTT_RES_CHECKED_JAL UINT32_C(0x0E271B28)
#define TTT_RES_UNCHECKED_JAL UINT32_C(0x0E271B58)

/* Read-only call sequence trace: JAL index 0..4 is the known resolver site,
 * NOT a player number. It is unsafe to infer P1..P4 from these calls alone. */
#define TTT_RES_TRACE_DEPTH 64u
typedef struct {
    uint32_t seq,site,id,offset,original,result,redirected;
} TTTResourceTrace;
uint32_t ttt_res_trace_count(void);
/* Returns 0 for a complete consistent event, -1 for missing/overwritten. */
int ttt_res_trace_get(uint32_t seq,TTTResourceTrace *out);

/* Match snapshot is updated ONLY by a request actually mapped to the
 * independent CHR. Subsequent ordinary calls cannot overwrite this data. */
typedef struct {
    int installed, last_error;
    uint32_t calls_checked,calls_unchecked,redirects;
    uint32_t last_id,last_variant,last_original,last_result;
    uint32_t match_count, match_id, match_offset, match_original, match_redirected;
    uint32_t match_path; /* 1=checked resolver, 2=unchecked resolver */
    uint32_t pack_magic,pack_count,resource_id,resource_size;
    uint32_t original_size;
    uint32_t resource_data_offset,original_data_offset;
} TTTResourceHookStats;

/* Pure validation for host-side testing. No changes to game memory. */
int ttt_res_make_jal(uint32_t site,uint32_t dest,uint32_t observed,
                      uint32_t expected,uint32_t *patched);
int ttt_res_validate_pack(uint32_t magic,uint32_t count,uint32_t resource_id,
                          uint32_t size);
/* Pure predicate shared by both resolver interceptors and host tests. */
/* Prototype safety check for a new last record pointing to existing Goku Base
 * bytes. No arbitrary new data is permitted in this first smoke test. */
int ttt_res_validate_alias(uint32_t new_offset,uint32_t original_offset,
                           uint32_t new_size,uint32_t original_size);
int ttt_res_should_redirect(int bridge_active,int id,int offset);
int ttt_res_hook_install(void);
void ttt_res_hook_remove(void);
TTTResourceHookStats ttt_res_hook_stats(void);
#endif
