#ifndef TTT_MENU_STATE_H
#define TTT_MENU_STATE_H
#include <stdint.h>
#define TTT_BTN_UP       UINT32_C(0x00000010)
#define TTT_BTN_RIGHT    UINT32_C(0x00000020)
#define TTT_BTN_DOWN     UINT32_C(0x00000040)
#define TTT_BTN_LEFT     UINT32_C(0x00000080)
#define TTT_BTN_L        UINT32_C(0x00000100)
#define TTT_BTN_R        UINT32_C(0x00000200)
#define TTT_BTN_TRIANGLE UINT32_C(0x00001000)
#define TTT_BTN_CIRCLE   UINT32_C(0x00002000)
#define TTT_BTN_CROSS    UINT32_C(0x00004000)
#define TTT_BTN_SQUARE   UINT32_C(0x00008000)
#define TTT_BTN_SELECT   UINT32_C(0x00000001)
#define TTT_EVENT_CONFIRM 1
#define TTT_EVENT_CLOSE 2
#define TTT_EVENT_BRIDGE_RESET 3
/* IDs externos de Tenkaichi 3, nao ativos no TTT original. */
#define TTT_ID_GOKU_EARLY 0u
#define TTT_ID_GOKU_MID   1u
#define TTT_ID_GOKU_MID_SSJ 2u
#define TTT_ID_GOKU_TTT 3u
/* A escolha sera apenas uma previa ate termos os dados internos independentes. */
typedef struct TTTMenuState {
    uint32_t previous;
    uint8_t character;       /* ativo: 0 Goku Early (ID 0), 1 Goku Mid (ID 1) */
    uint8_t color;           /* ativo: 0..3 */
    uint8_t tab,lang,player; /* 0..2, 0..2, 0..3 */
    uint8_t picked_character[4];
    uint8_t picked_color[4];
    uint8_t confirmed[4];   /* preview confirmado em cada slot, nao aplicado ao jogo */
} TTTMenuState;
void ttt_menu_init(TTTMenuState *m);
void ttt_menu_rebase(TTTMenuState *m,uint32_t buttons);
int ttt_menu_buttons(TTTMenuState *m,uint32_t buttons);
uint32_t ttt_menu_snapshot(const TTTMenuState *m);
uint8_t ttt_menu_external_id(const TTTMenuState *m);
#endif
