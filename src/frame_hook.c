/* TTT Expansion V0.5 - ULUS10537/EBOOT.OLD import-stub hook.
 * Patch only a known 8-byte user-memory syscall stub. On mismatch do NOT patch.
 * Trial-only: graphics commands inside SetFrameBuf can interfere with the game.
 * Never perform IO, print to file, or wait for vblank in the hooked function.
 */
#include <pspuser.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <stdint.h>
#include "framecheck.h"
#include "gpu_probe.h"
#include "frame_hook.h"
#define HOOK_ADDRESS UINT32_C(0x08A66A84)
#define PSP_JR_RA UINT32_C(0x03E00008)
#define PSP_NOP UINT32_C(0x00000000)
#define PSP_J_OP UINT32_C(0x08000000)
#define DRAW_LIMIT 180u /* 3 seconds at 60fps; then diagnostic stops drawing */

static volatile uint32_t saved_a, saved_b;
static volatile int hooked, gpu_ready, draw_allowed;
static volatile uint32_t call_count, draw_count, last_vcount, last_buffer;
static volatile int last_draw_rc;

static __attribute__((noinline)) int ttt_framebuf_intercept(void *fb,int stride,int fmt,int sync) {
    /* 'j intercept' in the EBOOT stub preserves the original MIPS $ra. */
    uint32_t vcount;
    ++call_count;
    last_buffer=(uint32_t)(uintptr_t)fb;
    if (hooked && gpu_ready && draw_allowed && draw_count<DRAW_LIMIT
        && ttt_vram_check((uintptr_t)fb,stride,fmt)==0) {
        vcount=(uint32_t)sceDisplayGetVcount();
        if (vcount!=last_vcount) {
            last_vcount=vcount;
            ++draw_count;
            last_draw_rc=ttt_gpu_frame(fb,stride,fmt);
            if(last_draw_rc<0) draw_allowed=0;
            if(draw_count>=DRAW_LIMIT) draw_allowed=0;
        }
    }
    /* IMPORTANT: call the plugin's own import, not the patched EBOOT stub. */
    return sceDisplaySetFrameBuf(fb,stride,fmt,sync);
}

static void flush_stub(void) {
    void *p=(void *)(uintptr_t)HOOK_ADDRESS;
    sceKernelDcacheWritebackInvalidateRange(p,8);
    sceKernelIcacheInvalidateRange(p,8);
}

TTTStubProbe ttt_frame_hook_probe(void) {
    /* Read only the four adjacent opcodes in the known ULUS10537 EBOOT stub. */
    volatile const uint32_t *base = (volatile const uint32_t *)(uintptr_t)UINT32_C(0x08A66A80);
    TTTStubProbe p;
    p.at_80=base[0]; p.at_84=base[1]; p.at_88=base[2]; p.at_8c=base[3];
    return p;
}

int ttt_frame_hook_install(void) {
    volatile uint32_t *stub=(volatile uint32_t *)(uintptr_t)HOOK_ADDRESS;
    uintptr_t target=(uintptr_t)&ttt_framebuf_intercept;
    uint32_t insn;
    if(hooked) return 0;
    if((target & UINT32_C(0xF0000000))!= ((HOOK_ADDRESS+4u)&UINT32_C(0xF0000000))) return -10;
    if((target&3u)!=0) return -11;
    /* Runtime import should be jr ra + syscall; EBOOT.OLD on disk has a nop
       for the second instruction, fixed up by the PPSSPP PSP loader. */
    if(stub[0]!=PSP_JR_RA) return -12;
    if(stub[1]!=PSP_NOP && (stub[1]&0x3fu)!=0x0cu) return -13;
    saved_a=stub[0]; saved_b=stub[1];
    insn=PSP_J_OP|((uint32_t)(target>>2)&UINT32_C(0x03ffffff));
    call_count=draw_count=last_vcount=last_buffer=0;
    last_draw_rc=0;
    draw_allowed=1;
    stub[1]=PSP_NOP;
    stub[0]=insn;
    flush_stub();
    hooked=1;
    return 0;
}
void ttt_frame_hook_remove(void) {
    volatile uint32_t *stub=(volatile uint32_t *)(uintptr_t)HOOK_ADDRESS;
    if(!hooked)return;
    draw_allowed=0;
    stub[0]=saved_a;
    stub[1]=saved_b;
    flush_stub();
    hooked=0;
}
void ttt_frame_hook_gpu_ready(int enabled) { gpu_ready=(enabled!=0); }
TTTFrameHookStats ttt_frame_hook_stats(void) {
    TTTFrameHookStats s;
    s.calls=call_count; s.draws=draw_count;
    s.last_buffer=last_buffer; s.last_vcount=last_vcount;
    s.last_draw_result=last_draw_rc;
    s.installed=hooked; s.draw_enabled=draw_allowed;
    return s;
}
