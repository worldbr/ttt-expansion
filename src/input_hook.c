/* TTT Expansion V0.6.1 input interception, ULUS10537 EBOOT.OLD only.
 * The game makes a known direct JAL to sceCtrlReadBufferPositive at 0881A158.
 * We patch that ONE call site; the plugin's own sceCtrlPeekBufferPositive is
 * never modified. Validate opcode before patch, restore only our own opcode.
 * No IO or logging within the game's controller polling callback.
 */
#include <pspctrl.h>
#include <psputils.h>
#include <stdint.h>
#include <stddef.h>
#include "input_hook.h"
#include "input_guard.h"

static volatile int hooked, menu_open, close_latch;
static volatile uint32_t old_opcode, our_opcode;
static volatile uint32_t calls, muted, last_raw, last_sent;
static TTTInputGuard guard;

static __attribute__((noinline)) int ttt_game_pad_intercept(SceCtrlData *pad,int count) {
    int i,ret=sceCtrlReadBufferPositive(pad,count);
    ++calls;
    if(close_latch){ ttt_input_guard_hold(&guard); close_latch=0; }
    if(ret>0 && count>0 && pad) {
        int upto=ret<count ? ret : count;
        if(upto>16)upto=16; /* Game site observed count=1. */
        for(i=0;i<upto;i++) {
            uint32_t raw=pad[i].Buttons;
            int silence=ttt_input_guard_mute(&guard,raw,menu_open);
            last_raw=raw;
            if(silence) {
                pad[i].Buttons=0;
                pad[i].Lx=128;
                pad[i].Ly=128;
                ++muted;
            }
            last_sent=pad[i].Buttons;
        }
    }
    return ret;
}

static void flush(void) {
    void *p=(void*)(uintptr_t)TTT_INPUT_SITE;
    sceKernelDcacheWritebackInvalidateRange(p,4);
    sceKernelIcacheInvalidateRange(p,4);
}
int ttt_input_hook_install(void) {
    uint32_t observed, patch;
    int rc;
    if(hooked)return 0;
    observed=*(volatile const uint32_t*)(uintptr_t)TTT_INPUT_SITE;
    rc=ttt_input_hook_plan(TTT_INPUT_SITE,(uint32_t)(uintptr_t)&ttt_game_pad_intercept,observed,&patch);
    if(rc<0)return rc;
    ttt_input_guard_init(&guard);
    menu_open=0;close_latch=0;
    calls=muted=last_raw=last_sent=0;
    old_opcode=observed;
    our_opcode=patch;
    *(volatile uint32_t*)(uintptr_t)TTT_INPUT_SITE=patch;
    flush();hooked=1;
    return 0;
}
void ttt_input_hook_remove(void) {
    volatile uint32_t*p=(volatile uint32_t*)(uintptr_t)TTT_INPUT_SITE;
    if(!hooked)return;
    menu_open=0;
    if(*p==our_opcode){*p=old_opcode;flush();}
    hooked=0;
}
void ttt_input_hook_menu(int is_open) {
    int prev=menu_open;
    menu_open=(is_open!=0);
    if(prev && !menu_open)close_latch=1;
}
void ttt_input_hook_hold_release(void) {close_latch=1;}
TTTInputHookStats ttt_input_hook_stats(void) {
    TTTInputHookStats st;
    st.calls=calls;st.muted=muted;st.last_raw=last_raw;st.last_sent=last_sent;
    st.site_opcode=*(volatile const uint32_t*)(uintptr_t)TTT_INPUT_SITE;
    st.target=(uint32_t)(uintptr_t)&ttt_game_pad_intercept;
    st.installed=hooked;st.quarantine=guard.quarantine;st.open=menu_open;
    return st;
}
