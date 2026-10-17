#ifndef TTT_MENU_STATE_H
#define TTT_MENU_STATE_H
#include <stdint.h>
/* PSP controller bit values. No PSPSDK dependencies: host-testable. */
#define TTT_BTN_UP       UINT32_C(0x00000010)
#define TTT_BTN_RIGHT    UINT32_C(0x00000020)
#define TTT_BTN_DOWN     UINT32_C(0x00000040)
#define TTT_BTN_LEFT     UINT32_C(0x00000080)
#define TTT_BTN_TRIANGLE UINT32_C(0x00001000)
#define TTT_BTN_CIRCLE   UINT32_C(0x00002000)
#define TTT_BTN_CROSS    UINT32_C(0x00004000)
#define TTT_BTN_SELECT   UINT32_C(0x00000001)
#define TTT_EVENT_CONFIRM 1
#define TTT_EVENT_CLOSE 2
typedef struct TTTMenuState {
    uint32_t previous;
    uint8_t character;  /* 0 = Goku Early, 1 = Goku Mid; two DIFFERENT extra entries, preview only */
    uint8_t color;      /* 0..3; preview, not applied to game */
    uint8_t tab;        /* 0 = characters, 1 = maps, 2 = settings */
    uint8_t lang;       /* 0 = PT, 1 = EN, 2 = ES */
} TTTMenuState;
void ttt_menu_init(TTTMenuState *m);
void ttt_menu_rebase(TTTMenuState *m,uint32_t buttons);
int ttt_menu_buttons(TTTMenuState *m,uint32_t buttons);
/* Packs one 32-bit snapshot for the GPU hook; writes are atomic on PSP/MIPS. */
uint32_t ttt_menu_snapshot(const TTTMenuState *m);
#endif
