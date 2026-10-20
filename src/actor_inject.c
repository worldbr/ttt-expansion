/* TTT Expansion V0.9.8: experimental four-slot ID/Color write interceptor.
 * Original EBOOT.OLD behavior preserved for non-enabled slots.
 * At PC 0x088576C8 game's original sw a0,0(fp) is replaced by jal shim;
 * 0x088576CC lw a0,0x18(sp) remains the jal delay slot, so the shim
 * retrieves the pre-delay ID from game's 0x20(sp).
 * PC 0x088576D0 then stores a1 to Color at 8(fp).
 * V0.9.8.6: experimental external ID00/01/02, Colors1-4.
 */
#include "actor_inject.h"
#include <stdint.h>
#ifdef TTT_BRIDGE_PSP
#include <pspuser.h>
#include <psputils.h>
#endif

/* Assembly-only read/write globals; 32-bit accesses are naturally aligned. */
volatile uint32_t ttt_ai_enabled_mask=0;
volatile uint32_t ttt_ai_requested_ids[4]={0,0,0,0};
volatile uint32_t ttt_ai_requested_colors[4]={0,0,0,0};
volatile uint32_t ttt_ai_hits_by_slot[4]={0,0,0,0};
volatile uint32_t ttt_ai_old_ids[4]={0,0,0,0};
volatile uint32_t ttt_ai_old_colors[4]={0,0,0,0};
volatile uint32_t ttt_ai_hits=0;
volatile uint32_t ttt_ai_last_player=0;
volatile uint32_t ttt_ai_last_original_id=0;
volatile uint32_t ttt_ai_last_original_color=0;
volatile uint32_t ttt_ai_last_requested_id=0;
volatile uint32_t ttt_ai_last_requested_color=0;
static volatile uint32_t g_patch=0,g_original=0,g_delay=0;
static volatile int g_installed=0,g_error=0;

int ttt_actor_inject_slot_for_address(uint32_t address){
    static const uint32_t ids[4]={TTT_ACTOR_P1_ID_ADDRESS,TTT_ACTOR_P2_ID_ADDRESS,TTT_ACTOR_P3_ID_ADDRESS,TTT_ACTOR_P4_ID_ADDRESS};
    int i;for(i=0;i<4;i++)if(address==ids[i])return i;
    return -1;
}
int ttt_actor_inject_should_replace(uint32_t address,uint32_t mask,uint32_t original_id){
    int slot=ttt_actor_inject_slot_for_address(address);
    return slot>=0 && (mask&(1u<<(uint32_t)slot))!=0u && original_id!=0u && original_id<=0xA0u;
}
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
extern void ttt_actor_inject_shim(void);
static void sync_opcode(uint32_t site){
    sceKernelDcacheWritebackInvalidateRange((void*)(uintptr_t)site,4);
    sceKernelIcacheInvalidateRange((void*)(uintptr_t)site,4);
}
#endif
int ttt_actor_inject_install(void){
#ifdef TTT_BRIDGE_PSP
    uint32_t a,b,patch;int rc;int i;
    if(g_installed){g_error=0;return 0;}
    a=*(volatile const uint32_t*)(uintptr_t)TTT_ACTOR_STORE_PC;
    b=*(volatile const uint32_t*)(uintptr_t)(TTT_ACTOR_STORE_PC+4u);
    rc=ttt_actor_inject_make_jal(TTT_ACTOR_STORE_PC,
        (uint32_t)(uintptr_t)&ttt_actor_inject_shim,a,b,&patch);
    if(rc){g_error=rc;return rc;}
    ttt_ai_enabled_mask=0;
    ttt_ai_hits=ttt_ai_last_player=0;
    ttt_ai_last_original_id=ttt_ai_last_original_color=0;
    ttt_ai_last_requested_id=ttt_ai_last_requested_color=0;
    for(i=0;i<4;i++){
        ttt_ai_requested_ids[i]=ttt_ai_requested_colors[i]=0;
        ttt_ai_hits_by_slot[i]=ttt_ai_old_ids[i]=ttt_ai_old_colors[i]=0;
    }
    g_original=a;g_delay=b;g_patch=patch;
    *(volatile uint32_t*)(uintptr_t)TTT_ACTOR_STORE_PC=patch;
    sync_opcode(TTT_ACTOR_STORE_PC);
    g_installed=1;g_error=0;
    return 0;
#else
    g_error=-90;return -90;
#endif
}
int ttt_actor_inject_enable_slot(int player,uint32_t external_id,uint32_t color){
    uint32_t bit;
    if(player<0||player>=4){g_error=-12;return -12;}
    if(!g_installed){g_error=-10;return -10;}
    if(external_id>2u || color>3u){g_error=-11;return -11;}
    bit=1u<<(uint32_t)player;
    /* Publish changes via mask LAST, so game thread sees complete plan. */
    ttt_ai_enabled_mask &= ~bit;
    ttt_ai_requested_ids[player]=external_id;
    ttt_ai_requested_colors[player]=color;
    ttt_ai_hits_by_slot[player]=0;
    ttt_ai_old_ids[player]=ttt_ai_old_colors[player]=0;
    ttt_ai_enabled_mask |= bit;
    g_error=0;return 0;
}
void ttt_actor_inject_disable_slot(int player){
    if(player>=0 && player<4)ttt_ai_enabled_mask &= ~(1u<<(uint32_t)player);
}
int ttt_actor_inject_enable(uint32_t id,uint32_t color){return ttt_actor_inject_enable_slot(0,id,color);}
void ttt_actor_inject_disable(void){ttt_ai_enabled_mask=0;}
void ttt_actor_inject_remove(void){
    ttt_actor_inject_disable();
#ifdef TTT_BRIDGE_PSP
    if(!g_installed)return;
    {
        volatile uint32_t *p=(volatile uint32_t*)(uintptr_t)TTT_ACTOR_STORE_PC;
        if(*p==g_patch){*p=g_original;sync_opcode(TTT_ACTOR_STORE_PC);g_error=0;}
        else g_error=-20;
    }
    g_installed=0;
#endif
}
TTTActorInjectStats ttt_actor_inject_stats(void){
    TTTActorInjectStats s;int i;
    s.installed=(uint32_t)g_installed;s.enabled_mask=ttt_ai_enabled_mask;
    s.enabled=(uint32_t)(ttt_ai_enabled_mask!=0);
    s.error=(uint32_t)g_error;s.original_insn=g_original;s.delay_insn=g_delay;s.patch_insn=g_patch;
    s.hits=ttt_ai_hits;s.last_player=ttt_ai_last_player;
    s.last_original_id=ttt_ai_last_original_id;s.last_original_color=ttt_ai_last_original_color;
    s.last_requested_id=ttt_ai_last_requested_id;s.last_requested_color=ttt_ai_last_requested_color;
    for(i=0;i<4;i++){
        s.slot_hits[i]=ttt_ai_hits_by_slot[i];
        s.slot_original_id[i]=ttt_ai_old_ids[i];s.slot_original_color[i]=ttt_ai_old_colors[i];
        s.slot_requested_id[i]=ttt_ai_requested_ids[i];s.slot_requested_color[i]=ttt_ai_requested_colors[i];
    }
    return s;
}
