#include <assert.h>
#include <stdio.h>
#include "menu_state.h"
static void release(TTTMenuState *m) {ttt_menu_buttons(m,0);}
int main(void) {
    TTTMenuState m;ttt_menu_init(&m);
    assert(m.form==0&&m.color==0&&m.tab==0&&m.lang==0);
    assert(ttt_menu_buttons(&m,TTT_BTN_RIGHT)==0&&m.form==1);
    assert(ttt_menu_buttons(&m,TTT_BTN_RIGHT)==0&&m.form==1);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_DOWN);assert(m.color==1);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_UP);assert(m.color==0);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_SELECT);assert(m.lang==1);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_TRIANGLE);assert(m.tab==1);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_LEFT);assert(m.form==1);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_TRIANGLE);assert(m.tab==2);
    release(&m);ttt_menu_buttons(&m,TTT_BTN_TRIANGLE);assert(m.tab==0);
    release(&m);assert(ttt_menu_buttons(&m,TTT_BTN_CROSS)==TTT_EVENT_CONFIRM);
    release(&m);assert(ttt_menu_buttons(&m,TTT_BTN_CIRCLE)==TTT_EVENT_CLOSE);
    assert(ttt_menu_snapshot(&m)==(1u|0u|(0u<<3)|(1u<<5)));
    ttt_menu_rebase(&m,TTT_BTN_RIGHT);
    assert(ttt_menu_buttons(&m,TTT_BTN_RIGHT)==0);
    puts("OK V0.6: navegacao, color, idioma, tabs, confirmacao e anti-repeticao");
    return 0;
}
