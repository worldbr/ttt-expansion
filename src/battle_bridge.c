/* TTT Expansion V0.9.5 -- Goku fifth-form carrier, NOT Videl.
 * EBOOT.OLD verified roster: 0,1,2,3,FF,0 at RAM 08A73F60.
 * New form 0x47 is an internal form index (71) not a replacement of ID03.
 * This grants a normal original-game selector route while we research truly
 * direct match entry from the custom menu (not provided in this version).
 */
#include "battle_bridge.h"
#include <stdint.h>
#ifdef TTT_BRIDGE_PSP
#include <pspuser.h>
#include <psputils.h>
#endif

static volatile int installed,last_error;
static volatile uint32_t installs,restores;
static volatile uint8_t old_a,old_b;
static volatile uint8_t old_a1,old_b1,old_a2,old_b2;
static volatile uint32_t old_slot,old_end,old_limit;

int ttt_bridge_validate(uint8_t a,uint8_t b,
                        const uint32_t goku_forms[6],uint32_t limit){
    int i;
    if(!goku_forms)return -10;
    if(a!=0xFF)return -11;
    if(b!=0xFF)return -12;
    for(i=0;i<4;++i)if(goku_forms[i]!=(uint32_t)i)return -16-i;
    if(goku_forms[4]!=TTT_BRIDGE_FORM_TERM)return -13;
    if(goku_forms[5]!=0u)return -14;
    if(limit!=TTT_BRIDGE_FORM_LIMIT_OLD)return -15;
    return 0;
}
#ifdef TTT_BRIDGE_PSP
static volatile uint8_t *p8(uint32_t x){return (volatile uint8_t*)(uintptr_t)x;}
static volatile uint32_t *p32(uint32_t x){return (volatile uint32_t*)(uintptr_t)x;}
static void flush32(uint32_t x){
    void *p=(void*)(uintptr_t)x;
    sceKernelDcacheWritebackInvalidateRange(p,4);
    sceKernelIcacheInvalidateRange(p,4);
}
static void flush8(uint32_t x){
    sceKernelDcacheWritebackInvalidateRange((void*)(uintptr_t)(x&~3u),4);
}
#endif
int ttt_bridge_install(void){
#ifdef TTT_BRIDGE_PSP
    uint8_t a,b;uint32_t forms[6],limit;int i,rc;
    if(installed){last_error=0;return 0;}
    if(*p8(TTT_BRIDGE_A_ID01)!=0xFF || *p8(TTT_BRIDGE_B_ID01)!=0xFF ||
       *p8(TTT_BRIDGE_A_ID02)!=0xFF || *p8(TTT_BRIDGE_B_ID02)!=0xFF){
        last_error=-23;return -23;
    }
    a=*p8(TTT_BRIDGE_A_ID00);
    b=*p8(TTT_BRIDGE_B_ID00);
    for(i=0;i<6;i++)forms[i]=*p32(TTT_BRIDGE_GOKU_GROUP+(uint32_t)i*4u);
    limit=*p32(TTT_BRIDGE_FORM_LIMIT);
    rc=ttt_bridge_validate(a,b,forms,limit);
    if(rc<0){last_error=rc;return rc;}
    old_a1=*p8(TTT_BRIDGE_A_ID01);old_b1=*p8(TTT_BRIDGE_B_ID01);
    old_a2=*p8(TTT_BRIDGE_A_ID02);old_b2=*p8(TTT_BRIDGE_B_ID02);
    old_a=a;old_b=b;old_slot=forms[4];old_end=forms[5];old_limit=limit;
    /* Set terminator first, then extend selection. Only Goku group is edited. */
    *p32(TTT_BRIDGE_GOKU_END)=TTT_BRIDGE_FORM_TERM;
    flush32(TTT_BRIDGE_GOKU_END);
    *p8(TTT_BRIDGE_A_ID00)=0u; /* Goku resource alias; Color1 CHR via resource hook */
    *p8(TTT_BRIDGE_B_ID00)=0x47u; /* external ID00 -> unique internal form 71 */
    *p8(TTT_BRIDGE_A_ID01)=0u;
    *p8(TTT_BRIDGE_A_ID02)=0u;
    *p8(TTT_BRIDGE_B_ID01)=0x47u; /* test: share proven internal form */
    *p8(TTT_BRIDGE_B_ID02)=0x47u;
    flush8(TTT_BRIDGE_A_ID00);flush8(TTT_BRIDGE_B_ID00);
    flush8(TTT_BRIDGE_A_ID01);flush8(TTT_BRIDGE_B_ID01);
    flush8(TTT_BRIDGE_A_ID02);flush8(TTT_BRIDGE_B_ID02);
    *p32(TTT_BRIDGE_FORM_LIMIT)=TTT_BRIDGE_FORM_LIMIT_NEW;
    flush32(TTT_BRIDGE_FORM_LIMIT);
    *p32(TTT_BRIDGE_GOKU_FORM5)=TTT_BRIDGE_FORM_EXTRA;
    flush32(TTT_BRIDGE_GOKU_FORM5);
    installed=1;last_error=0;++installs;
    return 0;
#else
    last_error=-90;return -90;
#endif
}
int ttt_bridge_remove(void){
#ifdef TTT_BRIDGE_PSP
    int changed=0;
    if(!installed){last_error=0;return 0;}
    /* Hide Goku fifth form before restoring indexes. Don't overwrite foreign edits. */
    if(*p32(TTT_BRIDGE_GOKU_FORM5)==TTT_BRIDGE_FORM_EXTRA){
        *p32(TTT_BRIDGE_GOKU_FORM5)=old_slot;flush32(TTT_BRIDGE_GOKU_FORM5);
    }else ++changed;
    if(*p32(TTT_BRIDGE_FORM_LIMIT)==TTT_BRIDGE_FORM_LIMIT_NEW){
        *p32(TTT_BRIDGE_FORM_LIMIT)=old_limit;flush32(TTT_BRIDGE_FORM_LIMIT);
    }else ++changed;
    if(*p8(TTT_BRIDGE_A_ID01)==0u){*p8(TTT_BRIDGE_A_ID01)=old_a1;flush8(TTT_BRIDGE_A_ID01);}else ++changed;
    if(*p8(TTT_BRIDGE_A_ID02)==0u){*p8(TTT_BRIDGE_A_ID02)=old_a2;flush8(TTT_BRIDGE_A_ID02);}else ++changed;
    if(*p8(TTT_BRIDGE_B_ID01)==0x47u){*p8(TTT_BRIDGE_B_ID01)=old_b1;flush8(TTT_BRIDGE_B_ID01);}else ++changed;
    if(*p8(TTT_BRIDGE_B_ID02)==0x47u){*p8(TTT_BRIDGE_B_ID02)=old_b2;flush8(TTT_BRIDGE_B_ID02);}else ++changed;
    if(*p8(TTT_BRIDGE_B_ID00)==0x47u){*p8(TTT_BRIDGE_B_ID00)=old_b;flush8(TTT_BRIDGE_B_ID00);}else ++changed;
    if(*p8(TTT_BRIDGE_A_ID00)==0u){*p8(TTT_BRIDGE_A_ID00)=old_a;flush8(TTT_BRIDGE_A_ID00);}else ++changed;
    if(*p32(TTT_BRIDGE_GOKU_END)==TTT_BRIDGE_FORM_TERM){
        *p32(TTT_BRIDGE_GOKU_END)=old_end;flush32(TTT_BRIDGE_GOKU_END);
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
#ifdef TTT_BRIDGE_PSP
    s.a1=*p8(TTT_BRIDGE_A_ID01);s.b1=*p8(TTT_BRIDGE_B_ID01);
    s.a2=*p8(TTT_BRIDGE_A_ID02);s.b2=*p8(TTT_BRIDGE_B_ID02);
    s.a=(uint32_t)*p8(TTT_BRIDGE_A_ID00);
    s.b=(uint32_t)*p8(TTT_BRIDGE_B_ID00);
    s.goku_5=*p32(TTT_BRIDGE_GOKU_FORM5);
    s.goku_end=*p32(TTT_BRIDGE_GOKU_END);
    s.limit=*p32(TTT_BRIDGE_FORM_LIMIT);
    s.videl_2=*p32(TTT_BRIDGE_VIDEL_FORM2);
    s.videl_end=*p32(TTT_BRIDGE_VIDEL_END);
#else
    s.a1=s.b1=s.a2=s.b2=0;
    s.a=s.b=s.goku_5=s.goku_end=s.limit=s.videl_2=s.videl_end=0;
#endif
    return s;
}
