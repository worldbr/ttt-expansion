#ifndef TTT_GPU_PROBE_H
#define TTT_GPU_PROBE_H
#include <stdint.h>
int ttt_gpu_init(void);
void ttt_gpu_set_menu(uint32_t snapshot);
int ttt_gpu_frame(void *fb, int stride, int psm);
void ttt_gpu_shutdown(void);
#endif
