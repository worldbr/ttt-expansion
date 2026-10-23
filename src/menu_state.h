#ifndef TTT_MENU_STATE_H
#define TTT_MENU_STATE_H
#include <stdint.h>
#define TTT_BTN_UP UINT32_C(0x10)
#define TTT_BTN_RIGHT UINT32_C(0x20)
#define TTT_BTN_DOWN UINT32_C(0x40)
#define TTT_BTN_LEFT UINT32_C(0x80)
#define TTT_BTN_L UINT32_C(0x100)
#define TTT_BTN_R UINT32_C(0x200)
#define TTT_BTN_TRIANGLE UINT32_C(0x1000)
#define TTT_BTN_CIRCLE UINT32_C(0x2000)
#define TTT_BTN_CROSS UINT32_C(0x4000)
#define TTT_BTN_SQUARE UINT32_C(0x8000)
#define TTT_BTN_SELECT UINT32_C(0x01)
#define TTT_MENU47_SLOTS 47
#define TTT_EVENT_CONFIRM 1
#define TTT_EVENT_CLOSE 2
#define TTT_EVENT_BRIDGE_RESET 3
#define TTT_EVENT_EMPTY 6
#define TTT_EVENT_SOUND_TOGGLE 7
typedef struct TTTMenuState {
    uint32_t previous;
    uint8_t character; /* 0..46: slot de tela */
    uint8_t color;     /* 0..3 */
    uint8_t focus;     /* 0 = grade, 1 = Colors */
    uint8_t player;    /* 0..3 */
    uint8_t sound;     /* 0 = sem musica, 1 = musica */
    uint8_t picked_character[4], picked_color[4], confirmed[4];
} TTTMenuState;
void ttt_menu_init(TTTMenuState *m);
void ttt_menu_rebase(TTTMenuState *m,uint32_t buttons);
int ttt_menu_buttons(TTTMenuState *m,uint32_t buttons);
uint32_t ttt_menu_snapshot(const TTTMenuState *m);
/* Slots 0,1,2 sao Goku Early/Mid/Mid SSJ; 3..46 aguardam IDs testados. */
int ttt_menu_slot_supported(unsigned slot);
uint8_t ttt_menu_external_id(const TTTMenuState *m);
#endif
