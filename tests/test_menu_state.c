#include <assert.h>
#include <stdio.h>
#include "menu_state.h"
static void release(TTTMenuState *m) {ttt_menu_buttons(m,0);}
int main(void) {
    TTTMenuState m;uint32_t snapshot;
    ttt_menu_init(&m);
    assert(m.player==0 && m.character==0 && m.color==0 && ttt_menu_external_id(&m)==0);
    /* Vertical list: Down -> Mid ID 01, Up -> Early ID 00 */
    assert(ttt_menu_buttons(&m,TTT_BTN_DOWN)==0 && m.character==1 && ttt_menu_external_id(&m)==1);
    release(&m);assert(ttt_menu_buttons(&m,TTT_BTN_UP)==0 && m.character==0);
    /* Horizontal color navigation */
    release(&m);ttt_menu_buttons(&m,TTT_BTN_RIGHT);assert(m.color==1);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_LEFT);assert(m.color==0);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_LEFT);assert(m.color==3);
    /* Player slots store independent character/color choices. */
    release(&m);ttt_menu_buttons(&m,TTT_BTN_DOWN);assert(m.character==1);
    release(&m);assert(ttt_menu_buttons(&m,TTT_BTN_CROSS)==TTT_EVENT_CONFIRM && m.confirmed[0]==1);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_R);assert(m.player==1 && m.character==0 && m.color==0);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_RIGHT);assert(m.color==1);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_R);assert(m.player==2);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_L);assert(m.player==1 && m.color==1);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_L);assert(m.player==0 && m.character==1 && m.color==3);
    snapshot=ttt_menu_snapshot(&m);
    assert((snapshot & 1u)==1u && ((snapshot>>7)&3u)==0);
    assert(((snapshot>>9)&1u)==1u);
    assert(((snapshot>>13)&7u)==(1u | (3u<<1))); /* P1 */
    assert(((snapshot>>16)&7u)==2u); /* P2 color 2 */
    release(&m);ttt_menu_buttons(&m,TTT_BTN_SELECT);assert(m.lang==1);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_TRIANGLE);assert(m.tab==1);
    release(&m);assert(ttt_menu_buttons(&m,TTT_BTN_CIRCLE)==TTT_EVENT_CLOSE);
    /* Square alone requests removal, but never as part of L+R+Square. */
    ttt_menu_init(&m);
    assert(ttt_menu_buttons(&m,TTT_BTN_SQUARE)==TTT_EVENT_BRIDGE_RESET);
    release(&m);
    assert(ttt_menu_buttons(&m,TTT_BTN_L|TTT_BTN_R|TTT_BTN_SQUARE)!=TTT_EVENT_BRIDGE_RESET);
    assert(TTT_ID_GOKU_EARLY==0 && TTT_ID_GOKU_MID==1 && TTT_ID_GOKU_MID_SSJ==2 && TTT_ID_GOKU_TTT==3);
    puts("OK V0.9.1: ID 0/1, navegacao vertical, colors horizontal, 4 jogadores, idiomas");
    return 0;
}
