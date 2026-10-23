#include <assert.h>
#include <stdio.h>
#include "menu_state.h"
static void release(TTTMenuState *m){ttt_menu_buttons(m,0u);}
static int press(TTTMenuState *m,uint32_t button){release(m);return ttt_menu_buttons(m,button);}
int main(void){
    TTTMenuState m;int i;uint32_t snap;
    ttt_menu_init(&m);
    assert(m.player==0 && m.character==0 && m.color==0 && m.focus==0);
    assert(press(&m,TTT_BTN_CROSS)==TTT_EVENT_CONFIRM);
    for(i=0;i<46;i++)press(&m,TTT_BTN_RIGHT);
    assert(m.character==46);
    assert(press(&m,TTT_BTN_CROSS)==TTT_EVENT_EMPTY);
    assert(ttt_menu_external_id(&m)==0xffu);
    assert(press(&m,TTT_BTN_RIGHT)==0 && m.character==0);
    assert(press(&m,TTT_BTN_DOWN)==0 && m.character==8);
    assert(press(&m,TTT_BTN_TRIANGLE)==0 && m.focus==1);
    assert(press(&m,TTT_BTN_LEFT)==0 && m.color==3);
    assert(press(&m,TTT_BTN_RIGHT)==0 && m.color==0);
    assert(press(&m,TTT_BTN_TRIANGLE)==0 && m.focus==0);
    assert(press(&m,TTT_BTN_R)==0 && m.player==1 && m.character==0);
    assert(press(&m,TTT_BTN_RIGHT)==0 && m.character==1);
    assert(press(&m,TTT_BTN_CROSS)==TTT_EVENT_CONFIRM && m.confirmed[1]);
    assert(press(&m,TTT_BTN_L)==0 && m.player==0 && m.character==8);
    assert(press(&m,TTT_BTN_SELECT)==TTT_EVENT_SOUND_TOGGLE && !m.sound);
    snap=ttt_menu_snapshot(&m);
    assert((snap & 63u)==8u && ((snap>>8)&3u)==0u && ((snap>>15)&1u)==0);
    assert(press(&m,TTT_BTN_SQUARE)==TTT_EVENT_BRIDGE_RESET);
    assert(press(&m,TTT_BTN_CIRCLE)==TTT_EVENT_CLOSE);
    for(i=0;i<47;i++)assert(ttt_menu_slot_supported(i)==(i<3));
    assert(!ttt_menu_slot_supported(47));
    puts("OK MENU47: grade 47, 4 players, 4 colors, bloqueio de 44 slots, audio toggle");
    return 0;
}
