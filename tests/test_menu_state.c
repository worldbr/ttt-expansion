#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "menu_state.h"
static void release(TTTMenuState *m){ttt_menu_buttons(m,0);}
int main(void){
 TTTMenuState m;int i;
 ttt_menu_init(&m);assert(m.character==0 && m.color==0 && ttt_menu_external_id(&m)==0);
 for(i=1;i<7;i++){ttt_menu_buttons(&m,TTT_BTN_DOWN);assert(m.character==i);release(&m);}
 assert(ttt_menu_external_id(&m)==0xFFu);
 assert(!strcmp(ttt_menu_character_name(6),"GOKU_GT_SSJ4"));
 ttt_menu_buttons(&m,TTT_BTN_DOWN);assert(m.character==0);release(&m);
 ttt_menu_buttons(&m,TTT_BTN_UP);assert(m.character==6);release(&m);
 ttt_menu_buttons(&m,TTT_BTN_UP);assert(m.character==5);release(&m);
 ttt_menu_buttons(&m,TTT_BTN_RIGHT);assert(m.color==1);release(&m);
 ttt_menu_buttons(&m,TTT_BTN_R);assert(m.player==1);release(&m);
 assert(ttt_menu_buttons(&m,TTT_BTN_CROSS)==TTT_EVENT_CONFIRM);
 assert((ttt_menu_snapshot(&m)&7u)==0 && ((ttt_menu_snapshot(&m)>>12)&1u)==1u);
 assert(ttt_menu_character_playable(0,0) && !ttt_menu_character_playable(1,0));
 release(&m);ttt_menu_buttons(&m,TTT_BTN_SELECT);assert(m.lang==1);release(&m);
 ttt_menu_buttons(&m,TTT_BTN_TRIANGLE);assert(m.tab==1);release(&m);
 assert(ttt_menu_buttons(&m,TTT_BTN_CIRCLE)==TTT_EVENT_CLOSE);
 ttt_menu_init(&m);assert(ttt_menu_buttons(&m,TTT_BTN_SQUARE)==TTT_EVENT_BRIDGE_RESET);
 puts("OK roster 7 slots / 4 colors / gameplay gating");return 0;
}
