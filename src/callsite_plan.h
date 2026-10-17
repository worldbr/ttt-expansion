#ifndef TTT_CALLSITE_PLAN_H
#define TTT_CALLSITE_PLAN_H
#include <stdint.h>

/* Build a direct JAL replacement without interpreting PPSSPP's HLE syscall stubs.
   Expected instruction is the original Tenkaichi Tag Team JAL to 08A66A84. */
int ttt_callsite_plan(uint32_t caller, uint32_t replacement,
                      uint32_t observed, uint32_t *new_opcode);
#endif
