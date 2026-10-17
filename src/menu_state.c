#include "menu_state.h"
static void save_active(TTTMenuState *m) {
    if(m->picked_character[m->player]!=m->character || m->picked_color[m->player]!=m->color)
        m->confirmed[m->player]=0;
    m->picked_character[m->player]=m->character;
    m->picked_color[m->player]=m->color;
}
static void change_player(TTTMenuState *m,int inc) {
    save_active(m);
    m->player=(uint8_t)((m->player+inc+4)%4);
    m->character=m->picked_character[m->player];
    m->color=m->picked_color[m->player];
}
void ttt_menu_init(TTTMenuState *m) {
    int i;
    m->previous=0;m->character=0;m->color=0;m->tab=0;m->lang=0;m->player=0;
    for(i=0;i<4;i++){m->picked_character[i]=0;m->picked_color[i]=0;m->confirmed[i]=0;}
}
void ttt_menu_rebase(TTTMenuState *m,uint32_t buttons) {m->previous=buttons;}
int ttt_menu_buttons(TTTMenuState *m,uint32_t buttons) {
    uint32_t pressed=buttons & ~m->previous;
    m->previous=buttons;
    if(pressed & TTT_BTN_CIRCLE) return TTT_EVENT_CLOSE;
    if(pressed & TTT_BTN_TRIANGLE) m->tab=(uint8_t)((m->tab+1u)%3u);
    if(pressed & TTT_BTN_SELECT) m->lang=(uint8_t)((m->lang+1u)%3u);
    if(m->tab==0) {
        /* L/R selecionam 1P..4P; apertar ambos nao muda jogador. */
        if((pressed & TTT_BTN_L) && !(buttons & TTT_BTN_R))change_player(m,-1);
        if((pressed & TTT_BTN_R) && !(buttons & TTT_BTN_L))change_player(m,1);
        if(pressed & TTT_BTN_UP) m->character=0;
        if(pressed & TTT_BTN_DOWN) m->character=1;
        if(pressed & TTT_BTN_LEFT) m->color=(uint8_t)((m->color+3u)%4u);
        if(pressed & TTT_BTN_RIGHT) m->color=(uint8_t)((m->color+1u)%4u);
        save_active(m);
        if(pressed & TTT_BTN_CROSS){m->confirmed[m->player]=1;return TTT_EVENT_CONFIRM;}
    }
    return 0;
}
uint8_t ttt_menu_external_id(const TTTMenuState *m) {
    return (uint8_t)(m->character?TTT_ID_GOKU_MID:TTT_ID_GOKU_EARLY);
}
uint32_t ttt_menu_snapshot(const TTTMenuState *m) {
    /* 0..0 char, 1..2 color, 3..4 tab, 5..6 lang, 7..8 player,
       9..12 confirm bits, 13..24 stored char(1)+color(2) each player. */
    uint32_t bits=(uint32_t)(m->character & 1u) | ((uint32_t)(m->color & 3u)<<1)
        | ((uint32_t)(m->tab & 3u)<<3) | ((uint32_t)(m->lang & 3u)<<5)
        | ((uint32_t)(m->player & 3u)<<7);
    int i;
    for(i=0;i<4;i++) {
        bits|=((uint32_t)(m->confirmed[i]&1u))<<(9+i);
        bits|=((uint32_t)(m->picked_character[i]&1u) | ((uint32_t)(m->picked_color[i]&3u)<<1)) << (13+i*3);
    }
    return bits;
}
