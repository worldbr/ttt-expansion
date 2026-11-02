#ifndef TTT_NATIVE_CODE_REPAIR_H
#define TTT_NATIVE_CODE_REPAIR_H
#include <stdint.h>
/* Verified ULUS10537 EBOOT V0.4 code image, not a generic pattern patch.
 * Return 0 when intact, 1 ONLY if the first checked word alone was lost,
 * -1 if any other word differs, -2 if the PACKFILE count is wrong. Read-only.
 * This does not execute or write into the PSP address space.
 */
#define TTT_V04_CHECKED_WORDS 24u
#define TTT_V04_UNCHECKED_WORDS 15u
/* First mismatching opcode for diagnostics. Null output arguments allowed.
 * Return: 0 exact; 1 first checked word only; -1 unsafe mismatch; -2 bad count.
 * module=0 checked, module=1 unchecked; index=word index. */
int ttt_v04_code_diagnose(const volatile uint32_t *checked,
                           const volatile uint32_t *unchecked,
                           uint32_t pack_count,
                           unsigned *module, unsigned *index,
                           uint32_t *actual, uint32_t *expected);
int ttt_v04_first_word_repairable(const volatile uint32_t *checked,
                                  const volatile uint32_t *unchecked,
                                  uint32_t pack_count);
/* Experimental V0.4.5: only indices 0 and 9 of the checked resolver may
 * be recovered, only after observing an intact EBOOT V0.4 in this process.
 * The observed overwritten data has the exact 0x682Bxxxx signature.
 * No other words, no unchecked resolver, no unknown code are patched. */
uint32_t ttt_v04_expected_checked_word(unsigned index);
int ttt_v04_repair_plan(const volatile uint32_t *checked,
                        const volatile uint32_t *unchecked,
                        uint32_t pack_count,int pristine_seen,
                        uint32_t *mask);
/* V0.4.7: 0 = pristine V0.4; 1 = read-only staged selection while game
 * overlays either resolver (requires known prefix, code sentinels, trusted
 * first battle, and separate per-ID mapping validation); negative = reject. */
int ttt_v04_resolver_phase(const volatile uint32_t *checked,
                           const volatile uint32_t *unchecked,
                           uint32_t count,int pristine_seen,int actor_hit_seen);
#endif
