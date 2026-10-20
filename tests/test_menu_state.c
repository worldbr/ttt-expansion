#include <assert.h>
#include <stdio.h>
#include "menu_state.h"
static void release(TTTMenuState *m){ttt_menu_buttons(m,0u);}
static int press(TTTMenuState *m,uint32_t button){int rc;release(m);rc=ttt_menu_buttons(m,button);return rc;}
int main(void){
    TTTMenuState m; uint32_t snap;
    ttt_menu_init(&m);
    assert(m.player==0&&m.character==0&&m.color==0&&m.focus==0);
    assert(press(&m,TTT_BTN_RIGHT)==0 && m.character==1);
    assert(press(&m,TTT_BTN_RIGHT)==0 && m.character==2);
    assert(ttt_menu_external_id(&m)==2);
    assert(press(&m,TTT_BTN_RIGHT)==0 && m.character==0);
    assert(press(&m,TTT_BTN_LEFT)==0 && m.character==2);
    assert(press(&m,TTT_BTN_DOWN)==0 && m.focus==1 && m.character==2);
    assert(press(&m,TTT_BTN_RIGHT)==0 && m.color==1 && m.character==2);
    assert(press(&m,TTT_BTN_LEFT)==0 && m.color==0);
    assert(press(&m,TTT_BTN_LEFT)==0 && m.color==3);
    assert((ttt_menu_snapshot(&m)>>31)==1u);
    assert(press(&m,TTT_BTN_CROSS)==TTT_EVENT_CONFIRM && m.confirmed[0]);
    assert(press(&m,TTT_BTN_R)==0 && m.player==1 && m.focus==0 && m.character==0);
    assert(press(&m,TTT_BTN_RIGHT)==0 && m.character==1);
    assert(press(&m,TTT_BTN_DOWN)==0 && m.focus==1);
    assert(press(&m,TTT_BTN_RIGHT)==0 && m.color==1);
    assert(press(&m,TTT_BTN_CROSS)==TTT_EVENT_CONFIRM && m.confirmed[1]);
    assert(press(&m,TTT_BTN_L)==0 && m.player==0 && m.character==2 && m.color==3 && m.confirmed[0]);
    assert(m.focus==0);
    snap=ttt_menu_snapshot(&m);
    assert((snap&3u)==2u && ((snap>>2)&3u)==3u && ((snap>>31)&1u)==0);
    assert(((snap>>14)&15u)==(2u | (3u<<2)));
    assert(((snap>>18)&15u)==(1u | (1u<<2)));
    /* confirm changed choice invalidates only that player */
    assert(press(&m,TTT_BTN_LEFT)==0 && m.character==1 && !m.confirmed[0] && m.confirmed[1]);
    assert(press(&m,TTT_BTN_UP)==0 && m.focus==0);
    assert(press(&m,TTT_BTN_SELECT)==0 && m.lang==1);
    assert(press(&m,TTT_BTN_TRIANGLE)==0 && m.tab==1 && m.focus==0);
    assert(press(&m,TTT_BTN_CIRCLE)==TTT_EVENT_CLOSE);
    ttt_menu_init(&m);
    assert(ttt_menu_buttons(&m,TTT_BTN_SQUARE)==TTT_EVENT_BRIDGE_RESET);
    release(&m);assert(ttt_menu_buttons(&m,TTT_BTN_L|TTT_BTN_R|TTT_BTN_SQUARE)!=TTT_EVENT_BRIDGE_RESET);
    assert(TTT_ID_GOKU_EARLY==0u&&TTT_ID_GOKU_MID==1u&&TTT_ID_GOKU_MID_SSJ==2u);
    puts("OK V1.0.1: grade horizontal + linha de cores + 1P-4P");
    return 0;
}
