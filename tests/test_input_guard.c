#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "input_guard.h"
#include "input_hook.h"

int main(void){
 TTTInputGuard g;uint32_t op=0;
 ttt_input_guard_init(&g);
 assert(!ttt_input_guard_mute(&g,0x20,0)); /* closed: arrow goes to game */
 assert(ttt_input_guard_mute(&g,TTT_INPUT_CHORD,0)); /* chord never leaks */
 assert(ttt_input_guard_mute(&g,0x300,0)); /* release square, hold L R */
 assert(ttt_input_guard_mute(&g,0x20,0)); /* still must release all */
 assert(!ttt_input_guard_mute(&g,0,0)); /* restored after neutral */
 assert(!ttt_input_guard_mute(&g,0x20,0));
 assert(ttt_input_guard_mute(&g,0x20,1)); /* menu open: neutral */
 assert(ttt_input_guard_mute(&g,0x4000,0)); /* X closing can't leak */
 assert(!ttt_input_guard_mute(&g,0,0));
 ttt_input_guard_hold(&g); /* closing via Circle or shortcut */
 assert(ttt_input_guard_mute(&g,0x2000,0));
 assert(!ttt_input_guard_mute(&g,0,0));
 assert(ttt_input_hook_plan(0x0881A158,0x08B6829C,0x0E299A9B,&op)==0);
 assert(op== (0x0C000000u|((0x08B6829Cu>>2)&0x03FFFFFFu)));
 assert(ttt_input_hook_plan(0x0881A158,0x08B6829C,0x0E299AA1,&op)==-12);
 assert(ttt_input_hook_plan(0x0881A158,0x18B6829C,0x0E299A9B,&op)==-10);
 puts("OK V0.6.1: bloqueio de botao / quarentena / assinatura JAL");
 return 0;
}
