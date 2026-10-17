#ifndef TTT_FRAMECHECK_H
#define TTT_FRAMECHECK_H
#include <stdint.h>
/* 0: valid. 1: null; 2: format invalid; 3: stride invalid; 4: outside VRAM; 5: rectangle exceeds VRAM */
int ttt_vram_check(uintptr_t addr, int stride, int fmt);
/* Alias uncached for VRAM; returns 0 for an address outside the first 2 MB. */
uintptr_t ttt_vram_uncached(uintptr_t addr);
#endif
