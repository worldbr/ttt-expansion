#ifndef TTT_GPU_PROBE_H
#define TTT_GPU_PROBE_H
#include <stdint.h>
/* All output is diagnostic; the plugin does not modify gameplay or character IDs. */
int ttt_gpu_init(void);
/* Validates a PSP VRAM framebuffer, emits an experimental GU list to it,
   waits for completion. Negative return indicates failure. */
int ttt_gpu_frame(void *fb, int stride, int psm);
void ttt_gpu_shutdown(void);
#endif
