#include "framecheck.h"
int ttt_vram_check(uintptr_t addr, int stride, int fmt) {
    uint32_t a=(uint32_t)addr;
    uint32_t offset,bytes,last;
    if(!a) return 1;
    if(fmt<0||fmt>3) return 2;
    if(stride<480||stride>2048) return 3;
    /* Mask CPU address aliases: cached 0x04XXXXXX, uncached 0x44XXXXXX, etc. */
    if((a & UINT32_C(0x1fe00000)) != UINT32_C(0x04000000)) return 4;
    offset=a & UINT32_C(0x001fffff);
    bytes=(fmt==3)?4u:2u;
    /* Only 480 visible pixels per scanline; last row y=271. */
    last=offset+(271u*(uint32_t)stride+480u)*bytes;
    if(last>UINT32_C(0x00200000)) return 5;
    return 0;
}

uintptr_t ttt_vram_uncached(uintptr_t addr) {
    /* PSP 0x04000000 and 0x44000000 map the same video RAM.
     * Uncached alias is essential for CPU-generated pixels to reach VRAM. */
    uint32_t a = (uint32_t)addr;
    if ((a & UINT32_C(0x1fe00000)) != UINT32_C(0x04000000)) return 0;
    return (uintptr_t)(UINT32_C(0x44000000) | (a & UINT32_C(0x001fffff)));
}
