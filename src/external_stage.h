#ifndef TTT_EXTERNAL_STAGE_H
#define TTT_EXTERNAL_STAGE_H
#include <stdint.h>
/* V1.0D: keep the complete external CHR in a private plugin RAM allocation.
 * The PSP game still loads from PACKFILE 1194. No loader pointers or metadata
 * are changed in this version. This is a staging / safety checkpoint only. */
#define TTT_STAGE_MAX_SIZE UINT32_C(0x00800000) /* 8 MiB PSP memory guard */
typedef struct {
    int result;
    uint32_t reference_size;
    uint32_t actual_size;
    uint32_t bytes_read;
    uint32_t first4;
    uint32_t fnv1a;
    uint32_t address;
    uint32_t retained;
    /* PSP: user partition, bypasses the small libc malloc heap.
       On host, this field is 2 (malloc) for portable tests. */
    uint32_t allocator;
    int32_t alloc_error; /* raw SceUID error when PSP allocation fails */
    int32_t block_uid; /* PSP allocated block UID when retained */
} TTTExternalStage;
int ttt_external_stage_load(const char *path, uint32_t reference, TTTExternalStage *out);
void ttt_external_stage_release(void);
const unsigned char *ttt_external_stage_data(void);
uint32_t ttt_external_stage_size(void);
#endif
