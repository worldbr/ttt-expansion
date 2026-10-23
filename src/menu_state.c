#include "menu_state.h"
static void save_active(TTTMenuState *m){
    int p=m->player;
    if(m->picked_character[p]!=m->character||m->picked_color[p]!=m->color)m->confirmed[p]=0;
    m->picked_character[p]=m->character;m->picked_color[p]=m->color;
}
int ttt_menu_slot_supported(unsigned slot){return slot<3u;}
uint8_t ttt_menu_external_id(const TTTMenuState *m){
    return ttt_menu_slot_supported(m->character)?m->character:0xffu;
}
void ttt_menu_init(TTTMenuState *m){
    int i;m->previous=0;m->character=0;m->color=0;m->focus=0;m->player=0;m->sound=1;
    for(i=0;i<4;i++){m->picked_character[i]=0;m->picked_color[i]=0;m->confirmed[i]=0;}
}
void ttt_menu_rebase(TTTMenuState *m,uint32_t buttons){m->previous=buttons;}
int ttt_menu_buttons(TTTMenuState *m,uint32_t buttons){
    uint32_t hit=buttons & ~m->previous;
    int ch;
    m->previous=buttons;
    if(hit & TTT_BTN_CIRCLE)return TTT_EVENT_CLOSE;
    if(hit & TTT_BTN_SELECT){m->sound^=1u;return TTT_EVENT_SOUND_TOGGLE;}
    if((hit & TTT_BTN_SQUARE) && !(buttons & (TTT_BTN_L|TTT_BTN_R)))return TTT_EVENT_BRIDGE_RESET;
    if((hit & TTT_BTN_L) && !(buttons & TTT_BTN_R)){
        save_active(m);m->player=(uint8_t)((m->player+3u)%4u);
        m->character=m->picked_character[m->player];m->color=m->picked_color[m->player];m->focus=0;
    }else if((hit & TTT_BTN_R) && !(buttons & TTT_BTN_L)){
        save_active(m);m->player=(uint8_t)((m->player+1u)%4u);
        m->character=m->picked_character[m->player];m->color=m->picked_color[m->player];m->focus=0;
    }
    ch=m->character;
    if(hit & TTT_BTN_TRIANGLE)m->focus^=1u;
    if(m->focus==0){
        if(hit & TTT_BTN_LEFT)ch=(ch+TTT_MENU47_SLOTS-1)%TTT_MENU47_SLOTS;
        if(hit & TTT_BTN_RIGHT)ch=(ch+1)%TTT_MENU47_SLOTS;
        if((hit & TTT_BTN_UP) && ch>=8)ch-=8;
        if((hit & TTT_BTN_DOWN) && ch+8<TTT_MENU47_SLOTS)ch+=8;
        m->character=(uint8_t)ch;
    }else{
        if(hit & (TTT_BTN_LEFT|TTT_BTN_UP))m->color=(uint8_t)((m->color+3u)%4u);
        if(hit & (TTT_BTN_RIGHT|TTT_BTN_DOWN))m->color=(uint8_t)((m->color+1u)%4u);
    }
    save_active(m);
    if(hit & TTT_BTN_CROSS){
        if(!ttt_menu_slot_supported(m->character)){m->confirmed[m->player]=0;return TTT_EVENT_EMPTY;}
        m->confirmed[m->player]=1;return TTT_EVENT_CONFIRM;
    }
    return 0;
}
/* 0..5 char; 6..7 color; 8..9 player; 10 focus; 11..14 confirmed;
 * 15 sound. Bit 30 remains legacy diagnostic bridge flag. */
uint32_t ttt_menu_snapshot(const TTTMenuState *m){
    uint32_t bits=(uint32_t)(m->character & 63u) |
        ((uint32_t)(m->color & 3u)<<6) |
        ((uint32_t)(m->player & 3u)<<8) |
        ((uint32_t)(m->focus & 1u)<<10) |
        ((uint32_t)(m->sound & 1u)<<15);
    int i;for(i=0;i<4;i++)bits|=(uint32_t)(m->confirmed[i]&1u)<<(11+i);
    return bits;
}
