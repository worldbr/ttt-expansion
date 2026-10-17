/* EXPERIMENTAL: PSP GE/GU overlay smoke-test for PPSSPP.
 * Never swaps display buffers and never calls sceDisplaySetFrameBuf.
 * Call only after a user opens the test panel, not at module_start.
 * This is NOT a stable graphics hook: game rendering may overwrite this.
 */
#include <pspgu.h>
#include <pspdisplay.h>
#include <stdint.h>
#include <stddef.h>
#include "framecheck.h"
#include "gpu_probe.h"

/* GU command lists require 16-byte alignment. Draw only flat sprites. */
static uint32_t __attribute__((aligned(16))) gu_list[4096];
static int gu_initialized = 0;

typedef struct TTTVertex { int16_t x, y, z; } TTTVertex;
static void rect(int x, int y, int w, int h, uint32_t color) {
    TTTVertex *v = (TTTVertex *)sceGuGetMemory(2 * sizeof(TTTVertex));
    if (!v) return;
    v[0].x=(int16_t)x; v[0].y=(int16_t)y; v[0].z=0;
    v[1].x=(int16_t)(x+w); v[1].y=(int16_t)(y+h); v[1].z=0;
    sceGuColor(color); /* PSP ABGR8888 */
    sceGuDrawArray(GU_SPRITES,GU_VERTEX_16BIT | GU_TRANSFORM_2D,2,NULL,v);
}

int ttt_gpu_init(void) {
    int rc;
    if (gu_initialized) return 0;
    rc=sceGuInit();
    if (rc>=0) gu_initialized=1;
    return rc;
}

int ttt_gpu_frame(void *fb,int stride,int fmt) {
    uintptr_t addr,offset;
    int start, finish, sync;
    if (!gu_initialized) return -10;
    if (ttt_vram_check((uintptr_t)fb,stride,fmt)!=0) return -11;
    addr=(uintptr_t)fb;
    offset=addr & (uintptr_t)0x003fffff;
    /* Drawing buffers are expressed as VRAM-relative offsets. */
    start=sceGuStart(GU_DIRECT,gu_list);
    if(start<0) return start;
    sceGuDrawBufferList(fmt,(void*)offset,stride);
    sceGuOffset(2048-240,2048-136);
    sceGuViewport(2048,2048,480,272);
    sceGuScissor(0,0,480,272);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuDisable(GU_TEXTURE_2D);
    sceGuDisable(GU_DEPTH_TEST);
    sceGuDisable(GU_LIGHTING);
    sceGuDisable(GU_CULL_FACE);
    sceGuDisable(GU_ALPHA_TEST);
    sceGuDisable(GU_BLEND);
    sceGuDisable(GU_STENCIL_TEST);
    sceGuDisable(GU_COLOR_TEST);
    sceGuDisable(GU_COLOR_LOGIC_OP);
    sceGuDisable(GU_FOG);
    sceGuDepthMask(1);
    sceGuShadeModel(GU_FLAT);
    /* Two deliberately large, high-contrast rectangles (not the full UI). */
    rect(20,18,320,58,0xFF7F350B);  /* deep blue */
    rect(20,18,320,8,0xFF00D9FF);   /* bright gold */
    rect(35,38,290,20,0xFF00D9FF);  /* bright gold */
    rect(95,103,290,94,0xFF7F350B);
    rect(95,103,290,8,0xFF00D9FF);
    rect(118,134,244,34,0xFF00D9FF);
    finish=sceGuFinish();
    if(finish<0) return finish;
    sync=sceGuSync(GU_SYNC_FINISH, GU_SYNC_WHAT_DONE);
    if(sync<0) return sync;
    return 0;
}

void ttt_gpu_shutdown(void) {
    /* Do not tear down GU callbacks while another game's GE list is running.
       The PRX lifetime is identical to the loaded game. */
}
