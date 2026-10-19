/* TTT Expansion V0.9.8.4F -- experimental CHR redirect to NEW PACKFILE
 * index 0x3711 (file 14098) on ULUS10537 ONLY. Requires expanded
 * header 0x3712 OR 0x3734 + correct encoded entry + matching EBOOT patch.
 * New record MUST alias original Goku Base file, to limit the first test.
 * The original game's loader still performs all file reads/decompression.
 * CPU hooks change only the returned numeric resource index.
 * No logs/filesystem I/O/allocations inside resolver interception.
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
static volatile uint32_t resource_data_offset,original_data_offset;
static volatile TTTResourceTrace traces[TTT_RES_TRACE_DEPTH];
static volatile uint32_t trace_count;
uint32_t ttt_res_trace_count(void){return trace_count;}
int ttt_res_trace_get(uint32_t seq,TTTResourceTrace *out){
    const volatile TTTResourceTrace *v;
    uint32_t count=trace_count;
    if(!out||seq==0||seq>count||(count-seq)>=TTT_RES_TRACE_DEPTH)return -1;
    v=&traces[(seq-1u)&(TTT_RES_TRACE_DEPTH-1u)];
    if(v->seq!=seq)return -1;
    out->seq=v->seq;out->site=v->site;out->id=v->id;out->offset=v->offset;
    out->original=v->original;out->result=v->result;out->redirected=v->redirected;
    return (v->seq==seq)?0:-1;
}
#ifdef TTT_BRIDGE_PSP
static void trace_event(uint32_t site,int id,int offset,int old,int result,int matched){
    uint32_t seq=trace_count+1u;
    volatile TTTResourceTrace *v=&traces[(seq-1u)&(TTT_RES_TRACE_DEPTH-1u)];
    v->seq=0;v->site=site;v->id=(uint32_t)id;v->offset=(uint32_t)offset;
    v->original=(uint32_t)old;v->result=(uint32_t)result;
    v->redirected=(uint32_t)matched;v->seq=seq;trace_count=seq;
}
#endif

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
    /* This release tests ONLY the new LAST entry, not the old 1194 slot.
     * NEVER return index 0x3711 on an original 0x3711-entry PACKFILE. */
    if(count!=UINT32_C(0x3712) && count!=UINT32_C(0x3734))return -11;
    if(id!=TTT_PACK_RESOURCE_EXTRA)return -12;
    if(size<UINT32_C(0x10000) || size>UINT32_C(0x02000000))return -13;
    return 0;
}
int ttt_res_validate_alias(uint32_t new_offset,uint32_t original_offset,
                           uint32_t new_size,uint32_t original_size){
    /* Ensure new record aliases the verified original Goku Base CHR.
     * Stop before enabling hooks if the newly appended record is malformed. */
    /* The RAM index stores the offset in the loader's native units, NOT as
     * a byte offset. The observed Goku CHR record is 0x0002258C, which
     * is a valid reference but is not 0x800-aligned as a numeric value.
     * Verify exact equality with the already validated original record.
     * Do NOT apply byte alignment tests to this RAM representation. */
    if(new_offset==0u || original_offset==0u || new_offset!=original_offset)return -14;
    if(new_size!=original_size)return -15;
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
static int intercept_common(int id,int offset,uint32_t site,int unchecked){
    int old,result,matched;
    result=((Resolver)(uintptr_t)(unchecked?UINT32_C(0x089C6D60):UINT32_C(0x089C6CA0)))(id,offset);
    old=result;
    if(unchecked)++calls_unchecked;else ++calls_checked;
    last_id=(uint32_t)id;last_variant=(uint32_t)offset;
    last_original=(uint32_t)old;
    matched=installed && ttt_res_should_redirect(ttt_bridge_is_active(),id,offset);
    if(matched){
        result=(int)TTT_PACK_RESOURCE_EXTRA;
        capture_match(id,offset,old,result,unchecked?2u:1u);
        ++redirects;
    }
    last_result=(uint32_t)result;
    trace_event(site,id,offset,old,result,matched);
    return result;
}
/* Separate wrappers preserve the exact call site for each lookup. The site
 * indicates which GAME FUNCTION asked, not which player owns the request. */
static __attribute__((noinline)) int check0(int id,int offset){return intercept_common(id,offset,0u,0);}
static __attribute__((noinline)) int check1(int id,int offset){return intercept_common(id,offset,1u,0);}
static __attribute__((noinline)) int check2(int id,int offset){return intercept_common(id,offset,2u,0);}
static __attribute__((noinline)) int uncheck3(int id,int offset){return intercept_common(id,offset,3u,1);}
static __attribute__((noinline)) int uncheck4(int id,int offset){return intercept_common(id,offset,4u,1);}

#endif
int ttt_res_hook_install(void){
#ifdef TTT_BRIDGE_PSP
    uint32_t old[SITE_N],patch[SITE_N];
    uint32_t target;int i,rc;
    if(installed){last_error=0;return 0;}
    pack_magic=*r32(TTT_PACK_HEADER);
    pack_count=*r32(TTT_PACK_HEADER+4u);
    /* Reject wrong header before reading beyond the original index table. */
    if(pack_magic!=UINT32_C(0x4B636150) || (pack_count!=UINT32_C(0x3712) && pack_count!=UINT32_C(0x3734))){
        resource_id=resource_size=original_size=0;
        resource_data_offset=original_data_offset=0;
        last_error=(pack_magic!=UINT32_C(0x4B636150))?-10:-11;
        return last_error;
    }
    resource_id=*r32(TTT_PACK_RESOURCE_EXTRA_RECORD+12u);
    resource_data_offset=*r32(TTT_PACK_RESOURCE_EXTRA_RECORD);
    resource_size=*r32(TTT_PACK_RESOURCE_EXTRA_RECORD+4u);
    original_data_offset=*r32(TTT_PACK_RESOURCE_GOKU_RECORD);
    original_size=*r32(TTT_PACK_RESOURCE_GOKU_RECORD+4u);
    rc=ttt_res_validate_pack(pack_magic,pack_count,resource_id,resource_size);
    if(!rc)rc=ttt_res_validate_alias(resource_data_offset,original_data_offset,resource_size,original_size);
    if(rc){last_error=rc;return rc;}
    for(i=0;i<SITE_N;i++){
        old[i]=*r32(callsites[i]);
        switch(i){
            case 0:target=(uint32_t)(uintptr_t)&check0;break;
            case 1:target=(uint32_t)(uintptr_t)&check1;break;
            case 2:target=(uint32_t)(uintptr_t)&check2;break;
            case 3:target=(uint32_t)(uintptr_t)&uncheck3;break;
            default:target=(uint32_t)(uintptr_t)&uncheck4;break;
        }
        rc=ttt_res_make_jal(callsites[i],target,old[i],expected[i],&patch[i]);
        if(rc){last_error=rc-100*(i+1);return last_error;}
    }
    calls_checked=calls_unchecked=redirects=0;
    last_id=last_variant=last_original=last_result=0;
    match_count=match_id=match_offset=match_original=match_redirected=match_path=0;
    trace_count=0;
    for(i=0;i<(int)TTT_RES_TRACE_DEPTH;i++)traces[i].seq=0;
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
    s.resource_data_offset=resource_data_offset;s.original_data_offset=original_data_offset;
    return s;
}
