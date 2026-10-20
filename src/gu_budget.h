#ifndef TTT_GU_BUDGET_H
#define TTT_GU_BUDGET_H
#include <stdint.h>
/*
 * PSP GU does NOT receive a buffer length in sceGuStart(), and
 * sceGuGetMemory() is not a capacity-checked allocation. A hard primitive
 * budget is mandatory: GU commands and vertex data share the same list.
 */
#define TTT_GU_LIST_WORDS (32768u)   /* 128 KiB: same allocation as bootable V0987. */
#define TTT_GU_MAX_PRIMITIVES (1200u) /* conservative GU command ceiling */
#define TTT_GU_CANARY_WORDS (16u)
#define TTT_GU_CANARY_VALUE UINT32_C(0xC0DECAFE)

typedef struct TTTGuBudget {
    uint32_t used;
    uint32_t skipped;
} TTTGuBudget;

static inline void ttt_gu_budget_begin(TTTGuBudget *b) {
    b->used = 0;
    b->skipped = 0;
}
static inline int ttt_gu_budget_take(TTTGuBudget *b) {
    if(b->used >= TTT_GU_MAX_PRIMITIVES) {
        ++b->skipped;
        return 0;
    }
    ++b->used;
    return 1;
}
/* The canaries detect corruption on both edges during testing. Prevention
 * comes from the conservative primitive budget plus a larger GU list. */
typedef struct TTTGuListStorage {
    uint32_t before[TTT_GU_CANARY_WORDS];
    uint32_t words[TTT_GU_LIST_WORDS];
    uint32_t after[TTT_GU_CANARY_WORDS];
} TTTGuListStorage;

static inline void ttt_gu_canaries_init(TTTGuListStorage *s) {
    unsigned i;
    for (i=0;i<TTT_GU_CANARY_WORDS;i++) {
        s->before[i]=TTT_GU_CANARY_VALUE;
        s->after[i]=TTT_GU_CANARY_VALUE;
    }
}
static inline int ttt_gu_canaries_ok(const TTTGuListStorage *s) {
    unsigned i;
    for (i=0;i<TTT_GU_CANARY_WORDS;i++) {
        if(s->before[i]!=TTT_GU_CANARY_VALUE ||
           s->after[i]!=TTT_GU_CANARY_VALUE) return 0;
    }
    return 1;
}
#endif
