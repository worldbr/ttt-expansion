/* Experimental Battle Bridge V0.9. First test ONLY: ID 00, 1P, Goku resources.
 *  The game has no internal character data for IDs 0..2. We borrow one menu
 *  internal form index (0x47) to preserve external ID 00, while resource map A
 *  points to existing Goku Base index 0. Real Early CHR/SFX are future work.
 *  We deliberately DO NOT modify the existing Goku ID 03, index 0, or resources.
 */
#include "battle_bridge.h"
#include <stdint.h>
#if defined(TTT_BRIDGE_PSP)
#include <pspuser.h>
#include <psputils.h>
#endif

static volatile int installed;
static volatile int last_error;
static volatile uint32_t installs,restores;
static volatile uint8_t old_a,old_b;
static volatile uint32_t old_videl2,old_end,old_limit;

int ttt_bridge_validate(uint8_t a,uint8_t b,uint32_t videl2,uint32_t end,uint32_t limit){
    if(a!=0xFF)return -11;
    if(b!=0xFF)return -12;
    if(videl2!=0x000000FFu)return -13;
    if(end!=0x00000000u)return -14;
    if(limit!=TTT_BRIDGE_FORM_LIMIT_OLD)return -15;
    return 0;
}
#if defined(TTT_BRIDGE_PSP)
static volatile uint8_t *addr8(uint32_t a){return (volatile uint8_t*)(uintptr_t)a;}
static volatile uint32_t *addr32(uint32_t a){return (volatile uint32_t*)(uintptr_t)a;}
static void flush32(uint32_t a){
    void*p=(void*)(uintptr_t)a;
    sceKernelDcacheWritebackInvalidateRange(p,4);
    sceKernelIcacheInvalidateRange(p,4);
}
static void flush8(uint32_t a){
    /* Round down to a word for cache helpers. */
    void*p=(void*)(uintptr_t)(a&~3u);
    sceKernelDcacheWritebackInvalidateRange(p,4);
}
#endif
int ttt_bridge_install(void){
#if defined(TTT_BRIDGE_PSP)
    uint8_t a,b;
    uint32_t v2,end,lim;
    int rc;
    if(installed){last_error=0;return 0;}
    a=*addr8(TTT_BRIDGE_A_ID00);
    b=*addr8(TTT_BRIDGE_B_ID00);
    v2=*addr32(TTT_BRIDGE_VIDEL_FORM2);
    end=*addr32(TTT_BRIDGE_VIDEL_END);
    lim=*addr32(TTT_BRIDGE_FORM_LIMIT);
    rc=ttt_bridge_validate(a,b,v2,end,lim);
    if(rc<0){last_error=rc;return rc;}
    old_a=a;old_b=b;old_videl2=v2;old_end=end;old_limit=lim;
    /* Install new terminator before making the second form visible. */
    *addr32(TTT_BRIDGE_VIDEL_END)=0x000000FFu;
    flush32(TTT_BRIDGE_VIDEL_END);
    *addr8(TTT_BRIDGE_A_ID00)=0u; /* CHR + SFX resource group: existing Goku Base */
    *addr8(TTT_BRIDGE_B_ID00)=0x47u; /* External 00 <-> internal form 71 */
    flush8(TTT_BRIDGE_A_ID00);flush8(TTT_BRIDGE_B_ID00);
    *addr32(TTT_BRIDGE_FORM_LIMIT)=TTT_BRIDGE_FORM_LIMIT_NEW;
    flush32(TTT_BRIDGE_FORM_LIMIT);
    *addr32(TTT_BRIDGE_VIDEL_FORM2)=0x00000047u;
    flush32(TTT_BRIDGE_VIDEL_FORM2);
    installed=1;last_error=0;++installs;
    return 0;
#else
    last_error=-90;return -90;
#endif
}
int ttt_bridge_remove(void){
#if defined(TTT_BRIDGE_PSP)
    int changed=0;
    if(!installed){last_error=0;return 0;}
    /* Restore the menu entry first. DO NOT overwrite changes made by others. */
    if(*addr32(TTT_BRIDGE_VIDEL_FORM2)==0x00000047u){
        *addr32(TTT_BRIDGE_VIDEL_FORM2)=old_videl2;flush32(TTT_BRIDGE_VIDEL_FORM2);
    }else ++changed;
    if(*addr32(TTT_BRIDGE_FORM_LIMIT)==TTT_BRIDGE_FORM_LIMIT_NEW){
        *addr32(TTT_BRIDGE_FORM_LIMIT)=old_limit;flush32(TTT_BRIDGE_FORM_LIMIT);
    }else ++changed;
    if(*addr8(TTT_BRIDGE_B_ID00)==0x47u){
        *addr8(TTT_BRIDGE_B_ID00)=old_b;flush8(TTT_BRIDGE_B_ID00);
    }else ++changed;
    if(*addr8(TTT_BRIDGE_A_ID00)==0u){
        *addr8(TTT_BRIDGE_A_ID00)=old_a;flush8(TTT_BRIDGE_A_ID00);
    }else ++changed;
    if(*addr32(TTT_BRIDGE_VIDEL_END)==0x000000FFu){
        *addr32(TTT_BRIDGE_VIDEL_END)=old_end;flush32(TTT_BRIDGE_VIDEL_END);
    }else ++changed;
    installed=0;last_error=changed?-21:0;++restores;
    return changed?-21:0;
#else
    last_error=-90;return -90;
#endif
}
int ttt_bridge_is_active(void){return installed;}
TTTBridgeStats ttt_bridge_stats(void){
    TTTBridgeStats s;
    s.installed=installed;s.last_error=last_error;s.installs=installs;s.restores=restores;
#if defined(TTT_BRIDGE_PSP)
    s.a=(uint32_t)*addr8(TTT_BRIDGE_A_ID00);
    s.b=(uint32_t)*addr8(TTT_BRIDGE_B_ID00);
    s.videl_2=*addr32(TTT_BRIDGE_VIDEL_FORM2);
    s.videl_end=*addr32(TTT_BRIDGE_VIDEL_END);
    s.limit=*addr32(TTT_BRIDGE_FORM_LIMIT);
#else
    s.a=s.b=s.videl_2=s.videl_end=s.limit=0;
#endif
    return s;
}
