#ifndef TTT_EXTERNAL_PROBE_H
#define TTT_EXTERNAL_PROBE_H
#include <stdint.h>
#define TTT_EXTERNAL_EARLY_COLOR1 "ms0:/PSP/PLUGINS/TTT_EXPANSION/characters/goku_early/color_1.pck1"
#define TTT_EXTERNAL_TEST_SIZE UINT32_C(0x00143000)
typedef struct {
    int result;
    uint32_t expected_size, actual_size, bytes_read, signature, fnv1a;
} TTTExternalProbe;
uint32_t ttt_probe_hash_update(uint32_t value,const unsigned char *data,uint32_t size);
int ttt_probe_validate_size(uint32_t expected,uint32_t actual);
/* READ-ONLY diagnostic; NEVER replaces PACKFILE game data. */
int ttt_external_probe_file(const char *path,uint32_t expected_size,TTTExternalProbe *report);
#endif
