#ifndef TTT_FRAME_HOOK_H
#define TTT_FRAME_HOOK_H
#include <stdint.h>
typedef struct TTTFrameHookStats {
    uint32_t calls, draws, last_buffer, last_vcount;
    int last_draw_result, installed, draw_enabled;
} TTTFrameHookStats;
typedef struct TTTStubProbe {
    uint32_t at_80, at_84, at_88, at_8c;
} TTTStubProbe;
TTTStubProbe ttt_frame_hook_probe(void);
int ttt_frame_hook_install(void);
void ttt_frame_hook_remove(void);
void ttt_frame_hook_gpu_ready(int enabled);
TTTFrameHookStats ttt_frame_hook_stats(void);
#endif
