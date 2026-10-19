/* ULUS10537 experimental 1P write interceptor.
 * Verified EBOOT.OLD:
 * 0x088576BC lw  a0,0x20(sp)  ; external ID from saved a3
 * 0x088576C0 lw  a1,0x1c(sp)  ; color from saved t0
 * 0x088576C8 sw  a0,0(fp)    ; actual 1P/2P/3P/4P actor ID store
 * 0x088576CC lw  a0,0x18(sp) ; must stay in JAL delay slot
 * 0x088576D0 sw  a1,0x8(fp)  ; Color store (shim overrides a1 only for 1P)
 *
 * The assembly shim duplicates the replaced sw, using original ID from
 * 0x20(sp) (since delay slot changes a0). This does NOT edit the actor
 * after resource loading, nor touch other players.
 */
#include "actor_inject.h"
#include <stdint.h>
#ifdef TTT_BRIDGE_PSP
#include <pspuser.h>
#include <psputils.h>
#endif

/* Symbols intentionally global for absolute relocations in actor_inject.S.
 * They are volatile because PPSSPP's game thread and plugin worker differ. */
volatile uint32_t ttt_ai_enabled=0;
volatile uint32_t ttt_ai_requested_id=0;
volatile uint32_t ttt_ai_requested_color=0;
volatile uint32_t ttt_ai_hits=0;
volatile uint32_t ttt_ai_last_original_id=0;
volatile uint32_t ttt_ai_last_original_color=0;
volatile uint32_t ttt_ai_last_requested_id=0;
volatile uint32_t ttt_ai_last_requested_color=0;
static volatile uint32_t g_patch=0,g_original=0,g_delay=0;
static volatile int g_installed=0,g_error=0;

int ttt_actor_inject_make_jal(uint32_t site,uint32_t target,uint32_t original,uint32_t delay,uint32_t *out){
    if(!out)return -1;
    if((site&3u)||(target&3u))return -2;
    if(((site+4u)&UINT32_C(0xF0000000))!=(target&UINT32_C(0xF0000000)))return -3;
    if(original!=TTT_ACTOR_STORE_EXPECTED)return -4;
    if(delay!=TTT_ACTOR_DELAY_EXPECTED)return -5;
    *out=UINT32_C(0x0C000000)|((target>>2)&UINT32_C(0x03FFFFFF));
    return 0;
}
#ifdef TTT_BRIDGE_PSP
extern void ttt_actor_inject_shim(void); /* Assembly-only, no compiler prologue. */
static void sync_opcode(uint32_t site){
    sceKernelDcacheWritebackInvalidateRange((void*)(uintptr_t)site,4);
    sceKernelIcacheInvalidateRange((void*)(uintptr_t)site,4);
}
#endif
int ttt_actor_inject_install(void){
#ifdef TTT_BRIDGE_PSP
    uint32_t a,b,patch;
    int rc;
    if(g_installed){g_error=0;return 0;}
    a=*(volatile const uint32_t*)(uintptr_t)TTT_ACTOR_STORE_PC;
    b=*(volatile const uint32_t*)(uintptr_t)(TTT_ACTOR_STORE_PC+4u);
    rc=ttt_actor_inject_make_jal(TTT_ACTOR_STORE_PC,
        (uint32_t)(uintptr_t)&ttt_actor_inject_shim,a,b,&patch);
    if(rc){g_error=rc;return rc;}
    ttt_ai_enabled=0;
    ttt_ai_hits=ttt_ai_last_original_id=ttt_ai_last_original_color=0;
    ttt_ai_last_requested_id=ttt_ai_last_requested_color=0;
    g_original=a;g_delay=b;g_patch=patch;
    *(volatile uint32_t*)(uintptr_t)TTT_ACTOR_STORE_PC=patch;
    sync_opcode(TTT_ACTOR_STORE_PC);
    g_installed=1;g_error=0;
    return 0;
#else
    g_error=-90;return -90;
#endif
}
int ttt_actor_inject_enable(uint32_t external_id,uint32_t color){
    /* First release deliberately supports ONLY ID00, 1P, Color1. */
    if(!g_installed){g_error=-10;return -10;}
    if(external_id!=0 || color!=0){g_error=-11;return -11;}
    ttt_ai_requested_id=external_id;
    ttt_ai_requested_color=color;
    ttt_ai_enabled=1;g_error=0;
    return 0;
}
void ttt_actor_inject_disable(void){ttt_ai_enabled=0;}
void ttt_actor_inject_remove(void){
    ttt_actor_inject_disable();
#ifdef TTT_BRIDGE_PSP
    if(!g_installed)return;
    {
        volatile uint32_t *p=(volatile uint32_t*)(uintptr_t)TTT_ACTOR_STORE_PC;
        if(*p==g_patch){*p=g_original;sync_opcode(TTT_ACTOR_STORE_PC);g_error=0;}
        else g_error=-20; /* Another patch changed this instruction: do not overwrite it. */
    }
    g_installed=0;
#endif
}
TTTActorInjectStats ttt_actor_inject_stats(void){
    TTTActorInjectStats s;
    s.installed=(uint32_t)g_installed;s.enabled=ttt_ai_enabled;
    s.error=(uint32_t)g_error;s.original_insn=g_original;s.delay_insn=g_delay;s.patch_insn=g_patch;
    s.hits=ttt_ai_hits;s.last_original_id=ttt_ai_last_original_id;
    s.last_original_color=ttt_ai_last_original_color;
    s.last_requested_id=ttt_ai_last_requested_id;
    s.last_requested_color=ttt_ai_last_requested_color;
    return s;
}
