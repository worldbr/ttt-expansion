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
    m->focus=0; /* sempre retorna a grade na troca de jogador */
}
void ttt_menu_init(TTTMenuState *m) {
    int i;
    m->previous=0;m->character=0;m->color=0;m->focus=0;m->tab=0;m->lang=0;m->player=0;
    for(i=0;i<4;i++){m->picked_character[i]=0;m->picked_color[i]=0;m->confirmed[i]=0;}
}
void ttt_menu_rebase(TTTMenuState *m,uint32_t buttons) {m->previous=buttons;}
int ttt_menu_buttons(TTTMenuState *m,uint32_t buttons) {
    uint32_t pressed=buttons & ~m->previous;
    m->previous=buttons;
    if(pressed & TTT_BTN_CIRCLE) return TTT_EVENT_CLOSE;
    if(pressed & TTT_BTN_TRIANGLE){m->tab=(uint8_t)((m->tab+1u)%3u);m->focus=0;}
    if(pressed & TTT_BTN_SELECT) m->lang=(uint8_t)((m->lang+1u)%3u);
    if(m->tab==0) {
        /* SQUARE (alone) removes the temporary ID00 battle bridge. */
        if((pressed & TTT_BTN_SQUARE) && !(buttons & (TTT_BTN_L|TTT_BTN_R)))
            return TTT_EVENT_BRIDGE_RESET;
        /* L/R selecionam 1P..4P; apertar ambos nao muda jogador. */
        if((pressed & TTT_BTN_L) && !(buttons & TTT_BTN_R))change_player(m,-1);
        if((pressed & TTT_BTN_R) && !(buttons & TTT_BTN_L))change_player(m,1);
        /* Navigacao espacial:
         * 3 personagens na primeira linha (←/→);
         * ↓ foca a fileira real C1-C4; ↑ retorna aos personagens.
         * Os cartoes FUTURO sao puramente decorativos e nao selecionaveis. */
        if(pressed & TTT_BTN_UP) m->focus=0;
        if(pressed & TTT_BTN_DOWN) m->focus=1;
        if(m->focus==0){
            if(pressed & TTT_BTN_LEFT) m->character=(uint8_t)((m->character+2u)%3u);
            if(pressed & TTT_BTN_RIGHT) m->character=(uint8_t)((m->character+1u)%3u);
        }else{
            if(pressed & TTT_BTN_LEFT) m->color=(uint8_t)((m->color+3u)%4u);
            if(pressed & TTT_BTN_RIGHT) m->color=(uint8_t)((m->color+1u)%4u);
        }
        save_active(m);
        if(pressed & TTT_BTN_CROSS){m->confirmed[m->player]=1;return TTT_EVENT_CONFIRM;}
    }
    return 0;
}
uint8_t ttt_menu_external_id(const TTTMenuState *m) {
    return m->character;
}
uint32_t ttt_menu_snapshot(const TTTMenuState *m) {
    /* V0986: char 0..1; color 2..3; tab 4..5; lang 6..7;
     * player 8..9; confirmed 10..13;
     * per-player (char 2 + color 2) at bits 14,18,22,26.
     * bit 30 = bridge active (ORed by worker); bit 31 = focus Colors. */
    uint32_t bits=(uint32_t)(m->character&3u) | ((uint32_t)(m->color&3u)<<2)
       | ((uint32_t)(m->tab&3u)<<4) | ((uint32_t)(m->lang&3u)<<6)
       | ((uint32_t)(m->player&3u)<<8);
    int i;
    for(i=0;i<4;i++) {
        bits|=((uint32_t)(m->confirmed[i]&1u))<<(10+i);
        bits|=(((uint32_t)(m->picked_character[i]&3u) |
                ((uint32_t)(m->picked_color[i]&3u)<<2)) << (14+i*4));
    }
    return bits | ((uint32_t)(m->focus&1u)<<31);
}
