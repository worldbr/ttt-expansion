#ifndef TTT_NATIVE_GUARD_H
#define TTT_NATIVE_GUARD_H
#include <stdint.h>

/* The V0.4 native resolver uses 47 reserved byte markers 0x80..0xAE
 * in the battle-ID -> selection table. Both lists were 0xFF originally.
 * This guard is deliberately independent of PSP SDK for host testing. */
#define TTT_V04_CHECKED0 UINT32_C(0x2C8800A1)
#define TTT_V04_CHECKED1 UINT32_C(0x15000007)
#define TTT_V04_UNCHECKED0 UINT32_C(0x2C8800A1)
#define TTT_V04_UNCHECKED1 UINT32_C(0x11000003)
#define TTT_V04_COUNT UINT32_C(0x37FC)

typedef struct TTTNativeGuard {
    uint32_t checked0, checked1, unchecked0, unchecked1, count;
    unsigned repaired_text, repaired_group;
    unsigned conflicts;
    unsigned bad_slot;
    uint8_t found_text, found_group;
    uint8_t expected_text;
    int error;
} TTTNativeGuard;

enum {
    TTT_NATIVE_VALID = 0,
    TTT_NATIVE_BAD_CODE = -1,
    TTT_NATIVE_BAD_COUNT = -2,
    TTT_NATIVE_CONFLICT = -3,
    TTT_NATIVE_VERIFY_FAILED = -4,
    TTT_NATIVE_BAD_SLOT = -5,
    TTT_NATIVE_PARTIAL = -6
};

/* Works exclusively on 47 IDs reserved in ULUS10537 V0.4.
 * Never writes if executable signature/count are wrong or a formerly
 * reserved ID has a non-FF unexpected value. Returns 0 on success.
 * Invalid mappings of 0xFF can be restored after a menu scene reload. */
int ttt_native_v04_ensure(TTTNativeGuard *st,
                          volatile uint8_t *text_table,
                          volatile uint8_t *group_table);
/* A chosen character can be safely repaired even if an unrelated slot
 * has an unknown mapping. Unknown bytes of the selected slot are NEVER
 * overwritten. Code signatures and pack count are always checked first. */
int ttt_native_v04_ensure_slot(TTTNativeGuard *st,
                               volatile uint8_t *text_table,
                               volatile uint8_t *group_table,
                               unsigned slot);
/* Repair only safe 0xFF entries for each of 47 slots independently.
 * Return PARTIAL if other slots are conflicting, leaving them unchanged.
 * A partial audit must not disable a verified, unrelated slot. */
int ttt_native_v04_repair_all(TTTNativeGuard *st,
                              volatile uint8_t *text_table,
                              volatile uint8_t *group_table);
/* Validate-only path when checked code was replaced by a different scene.
 * No writes, no code execution, no resources loaded. */
int ttt_native_v04_verify_mappings(TTTNativeGuard *st,
                                   volatile const uint8_t *text_table,
                                   volatile const uint8_t *group_table,
                                   unsigned slot,int audit_all);
#endif
