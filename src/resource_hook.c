/* V0.9.3 -- independent CHR resource diagnostics without changing redirect behavior.
 * The ID00 battle bridge preserves external ID00 and borrows Goku data.
 * We redirect ONLY requests to ID00, resource offset zero (Color 1)
 * from the game's verified CHR/SFX number resolver call sites to PACKFILE
 * entry 1194 (0x4A9, index starts at zero). The original resource loader
 * continues performing allocation, decompression, and file reads.
 *
 * Modifies 5 verified JALs ONLY after validating all 5 original opcodes and
 * the PACKFILE metadata. No logs/I/O/allocations in hooks. No external CHR.
 * Test on ULUS10537 original EBOOT.OLD, with user-modified PACKFILE ONLY.
 */
#include "resource_hook.h"
#include "battle_bridge.h"
#include <stdint.h>
#ifdef TTT_BRIDGE_PSP
#include <pspuser.h>
#include <psputils.h>
#endif

#define SITE_N 5
#ifdef TTT_BRIDGE_PSP
static const uint32_t callsites[SITE_N]={
 UINT32_C(0x0885520C),UINT32_C(0x08855440),UINT32_C(0x089AE3C0),
 UINT32_C(0x088551FC),UINT32_C(0x089AE3AC)
};
static const uint32_t expected[SITE_N]={
 TTT_RES_CHECKED_JAL,TTT_RES_CHECKED_JAL,TTT_RES_CHECKED_JAL,
 TTT_RES_UNCHECKED_JAL,TTT_RES_UNCHECKED_JAL
};
#endif
static volatile uint32_t saved[SITE_N],replacement[SITE_N];
static volatile int installed,last_error;
static volatile uint32_t calls_checked,calls_unchecked,redirects;
static volatile uint32_t last_id,last_variant,last_original,last_result;
static volatile uint32_t match_count,match_id,match_offset,match_original,match_redirected,match_path;
static volatile uint32_t pack_magic,pack_count,resource_id,resource_size,original_size;

int ttt_res_make_jal(uint32_t site,uint32_t dest,uint32_t observed,
                      uint32_t want,uint32_t *patched){
    if(!patched)return -1;
    if((site&3u)||(dest&3u))return -2;
    if(((site+4u)&UINT32_C(0xF0000000))!=(dest&UINT32_C(0xF0000000)))return -3;
    if(observed!=want)return -4;
    *patched=UINT32_C(0x0C000000)|((dest>>2)&UINT32_C(0x03FFFFFF));
    return 0;
}
int ttt_res_validate_pack(uint32_t magic,uint32_t count,uint32_t id,uint32_t size){
    if(magic!=UINT32_C(0x4B636150))return -10; /* PacK little endian */
    if(count!=UINT32_C(0x3711))return -11;
    if(id!=TTT_PACK_RESOURCE_EXTRA)return -12;
    if(size<UINT32_C(0x10000) || size>UINT32_C(0x02000000))return -13;
    return 0;
}
int ttt_res_should_redirect(int bridge_active,int id,int offset){
    return bridge_active && id==0 && offset==0;
}
#ifdef TTT_BRIDGE_PSP
static void capture_match(int id,int offset,int old_resource,int new_resource,uint32_t route){
    /* No allocations or filesystem I/O while intercepting resource resolver. */
    match_id=(uint32_t)id;
    match_offset=(uint32_t)offset;
    match_original=(uint32_t)old_resource;
    match_redirected=(uint32_t)new_resource;
    match_path=route;
    ++match_count; /* write last for a consistent snapshot when logging */
}
static void flush(uint32_t addr){
    void *p=(void*)(uintptr_t)addr;
    sceKernelDcacheWritebackInvalidateRange(p,4);
    sceKernelIcacheInvalidateRange(p,4);
}
static volatile const uint32_t *r32(uint32_t addr){return (volatile const uint32_t*)(uintptr_t)addr;}
/* Match game's two resource resolvers; no recursion because direct calls
 * jump into originals rather than the patched call sites. */
typedef int (*Resolver)(int external_id,int resource_offset);
static __attribute__((noinline)) int check_intercept(int id,int offset){
    int result=((Resolver)(uintptr_t)UINT32_C(0x089C6CA0))(id,offset);
    ++calls_checked;
    last_id=(uint32_t)id;last_variant=(uint32_t)offset;
    last_original=(uint32_t)result;
    if(installed && ttt_res_should_redirect(ttt_bridge_is_active(),id,offset)) {
        result=(int)TTT_PACK_RESOURCE_EXTRA;
        capture_match(id,offset,(int)last_original,result,1u);
        ++redirects;
    }
    last_result=(uint32_t)result;
    return result;
}
static __attribute__((noinline)) int uncheck_intercept(int id,int offset){
    int result=((Resolver)(uintptr_t)UINT32_C(0x089C6D60))(id,offset);
    ++calls_unchecked;
    last_id=(uint32_t)id;last_variant=(uint32_t)offset;
    last_original=(uint32_t)result;
    if(installed && ttt_res_should_redirect(ttt_bridge_is_active(),id,offset)){
        result=(int)TTT_PACK_RESOURCE_EXTRA;
        capture_match(id,offset,(int)last_original,result,2u);
        ++redirects;
    }
    last_result=(uint32_t)result;
    return result;
}
#endif
int ttt_res_hook_install(void){
#ifdef TTT_BRIDGE_PSP
    uint32_t old[SITE_N],patch[SITE_N];
    uint32_t target;int i,rc;
    if(installed){last_error=0;return 0;}
    pack_magic=*r32(TTT_PACK_HEADER);
    pack_count=*r32(TTT_PACK_HEADER+4u);
    resource_id=*r32(TTT_PACK_RESOURCE_EXTRA_RECORD+12u);
    resource_size=*r32(TTT_PACK_RESOURCE_EXTRA_RECORD+4u);
    original_size=*r32(TTT_PACK_RESOURCE_GOKU_RECORD+4u);
    rc=ttt_res_validate_pack(pack_magic,pack_count,resource_id,resource_size);
    if(rc){last_error=rc;return rc;}
    for(i=0;i<SITE_N;i++){
        old[i]=*r32(callsites[i]);
        target=(uint32_t)(uintptr_t)(i<3?&check_intercept:&uncheck_intercept);
        rc=ttt_res_make_jal(callsites[i],target,old[i],expected[i],&patch[i]);
        if(rc){last_error=rc-100*(i+1);return last_error;}
    }
    calls_checked=calls_unchecked=redirects=0;
    last_id=last_variant=last_original=last_result=0;
    match_count=match_id=match_offset=match_original=match_redirected=match_path=0;
    for(i=0;i<SITE_N;i++){
        saved[i]=old[i];replacement[i]=patch[i];
        *(volatile uint32_t*)(uintptr_t)callsites[i]=patch[i];
        flush(callsites[i]);
    }
    installed=1;last_error=0;
    return 0;
#else
    last_error=-90;return -90;
#endif
}
void ttt_res_hook_remove(void){
#ifdef TTT_BRIDGE_PSP
    int i;
    if(!installed)return;
    installed=0;
    for(i=0;i<SITE_N;i++){
        volatile uint32_t *p=(volatile uint32_t*)(uintptr_t)callsites[i];
        if(*p==replacement[i]){*p=saved[i];flush(callsites[i]);}
    }
#endif
}
TTTResourceHookStats ttt_res_hook_stats(void){
    TTTResourceHookStats s;
    s.installed=installed;s.last_error=last_error;
    s.calls_checked=calls_checked;s.calls_unchecked=calls_unchecked;
    s.redirects=redirects;s.last_id=last_id;s.last_variant=last_variant;
    s.last_original=last_original;s.last_result=last_result;
    s.match_count=match_count;s.match_id=match_id;s.match_offset=match_offset;
    s.match_original=match_original;s.match_redirected=match_redirected;
    s.match_path=match_path;
    s.pack_magic=pack_magic;s.pack_count=pack_count;
    s.resource_id=resource_id;s.resource_size=resource_size;s.original_size=original_size;
    return s;
}
