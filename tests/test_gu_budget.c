#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "gu_budget.h"
int main(void) {
    TTTGuBudget b;
    TTTGuListStorage *s;
    unsigned i;
    ttt_gu_budget_begin(&b);
    for(i=0;i<TTT_GU_MAX_PRIMITIVES;i++)assert(ttt_gu_budget_take(&b));
    for(i=0;i<9;i++)assert(!ttt_gu_budget_take(&b));
    assert(b.used==TTT_GU_MAX_PRIMITIVES && b.skipped==9);
    ttt_gu_budget_begin(&b);
    assert(b.used==0 && b.skipped==0 && ttt_gu_budget_take(&b));
    assert(TTT_GU_LIST_WORDS == 32768u);
    s=malloc(sizeof(*s));assert(s);
    memset(s,0,sizeof(*s));
    ttt_gu_canaries_init(s);
    assert(ttt_gu_canaries_ok(s));
    s->after[0]=0;
    assert(!ttt_gu_canaries_ok(s));
    ttt_gu_canaries_init(s);
    s->before[TTT_GU_CANARY_WORDS-1]=0;
    assert(!ttt_gu_canaries_ok(s));
    free(s);
    return 0;
}
