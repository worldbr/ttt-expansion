#ifndef TTT_GPU_PROBE_H
#define TTT_GPU_PROBE_H
#include <stdint.h>
typedef struct TTTGeGuardStats {
    int init_save, init_restore, frame_save, frame_restore;
    unsigned attempts, restores;
    int failed;
    unsigned primitives, peak_primitives, skipped_primitives, list_bytes;
    int canary_fault;
} TTTGeGuardStats;
TTTGeGuardStats ttt_gpu_guard_stats(void);
int ttt_gpu_init(void);
void ttt_gpu_set_menu(uint32_t snapshot);
void ttt_gpu_set_dlc(unsigned mission, uint32_t completed);
int ttt_gpu_frame(void *fb, int stride, int psm);
void ttt_gpu_shutdown(void);
/* Bits: early=1, mid=2, images loaded from ms0:/PSP/PLUGINS. */
int ttt_gpu_portrait_status(void);
/* 1: HD skin loaded from external raw textures, 0: legacy fallback. */
int ttt_gpu_skin_status(void);
/* stage: 0 success, 1 open error, 2 wrong size/seek, 3 memory allocation,
 * 4 null memory block pointer, 5 short read,
 * 6 background unavailable (shared atlas not possible). */
typedef struct {
    int stage,open_rc,file_size,alloc_rc,read_bytes;
} TTTSkinFileStats;
typedef struct {
    TTTSkinFileStats bg,font;
    unsigned mask;
} TTTSkinStats;
TTTSkinStats ttt_gpu_skin_stats(void);
#endif
