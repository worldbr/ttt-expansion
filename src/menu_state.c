#include "menu_state.h"
void ttt_menu_init(TTTMenuState *m) {
    m->previous=0;m->form=0;m->color=0;m->tab=0;m->lang=0;
}
void ttt_menu_rebase(TTTMenuState *m,uint32_t buttons) {m->previous=buttons;}
int ttt_menu_buttons(TTTMenuState *m,uint32_t buttons) {
    uint32_t pressed=buttons & ~m->previous;
    m->previous=buttons;
    if(pressed & TTT_BTN_CIRCLE) return TTT_EVENT_CLOSE;
    if(pressed & TTT_BTN_TRIANGLE) m->tab=(uint8_t)((m->tab+1u)%3u);
    if(pressed & TTT_BTN_SELECT) m->lang=(uint8_t)((m->lang+1u)%3u);
    if(m->tab==0) {
        if(pressed & TTT_BTN_LEFT) m->form=0;
        if(pressed & TTT_BTN_RIGHT) m->form=1;
        if(pressed & TTT_BTN_UP) m->color=(uint8_t)((m->color+3u)%4u);
        if(pressed & TTT_BTN_DOWN) m->color=(uint8_t)((m->color+1u)%4u);
        if(pressed & TTT_BTN_CROSS) return TTT_EVENT_CONFIRM;
    }
    return 0;
}
uint32_t ttt_menu_snapshot(const TTTMenuState *m) {
    return (uint32_t)(m->form & 1u) | ((uint32_t)(m->color & 3u)<<1)
        | ((uint32_t)(m->tab & 3u)<<3) | ((uint32_t)(m->lang & 3u)<<5);
}
