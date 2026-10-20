/* TTT Expansion V0.5.2 -- known-call-site hook, not an import-stub hook.
 * For ULUS10537 / EBOOT.OLD original game code ONLY.
 * PPSSPP patches the loaded sceDisplaySetFrameBuf import stub with HLE
 * instructions (observed 0x6801AFE2 at 08A66A84). DO NOT PATCH THAT STUB.
 * Instead, redirect THREE verified JAL call sites in the EBOOT code.
 * Trial-only: GU commands within SetFrameBuf call path may glitch or hang.
 * V0.5.3: stay active until user closes the menu; draw max once per vcount.
 * No IO, log writes or VBlank waits inside the hook itself.
 */
#include <pspuser.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <stdint.h>
#include "framecheck.h"
#include "gpu_probe.h"
#include "callsite_plan.h"
#include "frame_hook.h"

#define SITE_COUNT 3
static const uint32_t site_addresses[SITE_COUNT] = {
    UINT32_C(0x08A63A48), UINT32_C(0x08A63B10), UINT32_C(0x08A6408C)
};
static volatile uint32_t saved[SITE_COUNT];
static volatile uint32_t our_jal;
static volatile int hooked, gpu_ready, draw_allowed;
static volatile uint32_t call_count, draw_count, last_vcount, last_buffer;
static volatile int last_draw_rc;
static volatile uint32_t retry_count;

static __attribute__((noinline)) int ttt_framebuf_intercept(void *fb,int stride,int fmt,int sync) {
    uint32_t vcount;
    ++call_count;
    last_buffer=(uint32_t)(uintptr_t)fb;
    if (hooked && gpu_ready && draw_allowed
        && ttt_vram_check((uintptr_t)fb,stride,fmt)==0) {
        vcount=(uint32_t)sceDisplayGetVcount();
        if(vcount!=last_vcount) {
            last_vcount=vcount;
            ++draw_count;
            last_draw_rc=ttt_gpu_frame(fb,stride,fmt);
            if(last_draw_rc<0) draw_allowed=0; /* GPU errors still stop drawing. */
        }
    }
    /* The PRX has an independent import; none of the game import stubs change. */
    return sceDisplaySetFrameBuf(fb,stride,fmt,sync);
}

static void flush_site(uint32_t addr) {
    void *p=(void *)(uintptr_t)addr;
    sceKernelDcacheWritebackInvalidateRange(p,4);
    sceKernelIcacheInvalidateRange(p,4);
}

TTTStubProbe ttt_frame_hook_probe(void) {
    volatile const uint32_t *b=(volatile const uint32_t *)(uintptr_t)UINT32_C(0x08A66A80);
    TTTStubProbe p;
    p.at_80=b[0];p.at_84=b[1];p.at_88=b[2];p.at_8c=b[3];
    return p;
}
TTTCallsProbe ttt_frame_hook_probe_calls(void) {
    TTTCallsProbe p;
    p.site_a=*(volatile const uint32_t *)(uintptr_t)site_addresses[0];
    p.site_b=*(volatile const uint32_t *)(uintptr_t)site_addresses[1];
    p.site_c=*(volatile const uint32_t *)(uintptr_t)site_addresses[2];
    p.replacement=(uint32_t)(uintptr_t)&ttt_framebuf_intercept;
    return p;
}

int ttt_frame_hook_install(void) {
    uint32_t target=(uint32_t)(uintptr_t)&ttt_framebuf_intercept;
    uint32_t patch=0;
    uint32_t previous[SITE_COUNT];
    int i,rc;
    if(hooked) return 0;
    /* Validate ALL original JALs before altering any instruction. */
    for(i=0;i<SITE_COUNT;i++) {
        previous[i]=*(volatile const uint32_t *)(uintptr_t)site_addresses[i];
        rc=ttt_callsite_plan(site_addresses[i],target,previous[i],&patch);
        if(rc<0) return rc - (i * 100); /* identifies first failing site */
    }
    call_count=draw_count=last_vcount=last_buffer=0;
    retry_count=0;
    last_draw_rc=0; draw_allowed=1;
    our_jal=patch;
    for(i=0;i<SITE_COUNT;i++) {
        saved[i]=previous[i];
        *(volatile uint32_t *)(uintptr_t)site_addresses[i]=patch;
        flush_site(site_addresses[i]);
    }
    hooked=1;
    return 0;
}
void ttt_frame_hook_remove(void) {
    int i;
    if(!hooked)return;
    draw_allowed=0;
    for(i=0;i<SITE_COUNT;i++) {
        volatile uint32_t *p=(volatile uint32_t *)(uintptr_t)site_addresses[i];
        /* Do not clobber an instruction modified by another component. */
        if(*p==our_jal) {
            *p=saved[i];
            flush_site(site_addresses[i]);
        }
    }
    hooked=0;
}
/* Worker-thread only: restart a halted render after a transient GU error.
 * Never bypass a failed GE context guard. Bounded retries avoid loops. */
int ttt_frame_hook_resume_drawing(void) {
    if(!hooked || !gpu_ready) return -1;
    if(draw_allowed) return 1;
    if(ttt_gpu_guard_stats().failed) return -2;
    if(retry_count>=2u) return -3;
    ++retry_count;
    last_vcount=UINT32_C(0xffffffff);
    last_draw_rc=0;
    draw_allowed=1;
    return 0;
}
void ttt_frame_hook_gpu_ready(int enabled) {gpu_ready=(enabled!=0);}
TTTFrameHookStats ttt_frame_hook_stats(void) {
    TTTFrameHookStats s;
    s.calls=call_count;s.draws=draw_count;
    s.last_buffer=last_buffer;s.last_vcount=last_vcount;
    s.last_draw_result=last_draw_rc;
    s.installed=hooked;s.draw_enabled=draw_allowed;
    return s;
}
