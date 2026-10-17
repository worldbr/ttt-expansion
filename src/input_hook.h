#ifndef TTT_INPUT_HOOK_H
#define TTT_INPUT_HOOK_H
#include <stdint.h>
#define TTT_INPUT_SITE UINT32_C(0x0881A158)
#define TTT_INPUT_ORIGINAL_JAL UINT32_C(0x0E299A9B) /* jal 0x08A66A6C */
typedef struct TTTInputHookStats {
    uint32_t calls, muted, last_raw, last_sent, site_opcode, target;
    int installed, quarantine, open;
} TTTInputHookStats;
int ttt_input_hook_plan(uint32_t caller,uint32_t replacement,uint32_t observed,uint32_t *out);
int ttt_input_hook_install(void);
void ttt_input_hook_remove(void);
void ttt_input_hook_menu(int is_open);
/* Called on close (including Circle), so the closing button never leaks. */
void ttt_input_hook_hold_release(void);
TTTInputHookStats ttt_input_hook_stats(void);
#endif
