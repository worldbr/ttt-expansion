#ifndef TTT_TOGGLE_H
#define TTT_TOGGLE_H
#include <stdint.h>
#define TTT_LTRIGGER UINT32_C(0x00000100)
#define TTT_RTRIGGER UINT32_C(0x00000200)
#define TTT_SQUARE   UINT32_C(0x00008000)
#define TTT_HOTKEY (TTT_LTRIGGER|TTT_RTRIGGER|TTT_SQUARE)

typedef struct { uint32_t previous; uint8_t open; } TTTToggle;
void ttt_toggle_init(TTTToggle *t);
/* Returns 1 on a new chord press, does not repeat while held. */
int ttt_toggle_frame(TTTToggle *t, uint32_t buttons);
#endif
