#include "callsite_plan.h"
#define TTT_ORIGINAL_JAL UINT32_C(0x0E299AA1) /* jal 0x08A66A84 */
#define MIPS_JAL UINT32_C(0x0C000000)

int ttt_callsite_plan(uint32_t caller, uint32_t replacement,
                      uint32_t observed, uint32_t *new_opcode) {
    if (!new_opcode) return -14;
    if ((caller & 3u) || (replacement & 3u)) return -11;
    /* MIPS direct J/JAL uses high nibble of PC+4. */
    if (((caller + 4u) & UINT32_C(0xF0000000)) !=
        (replacement & UINT32_C(0xF0000000))) return -10;
    if (observed != TTT_ORIGINAL_JAL) return -12;
    *new_opcode = MIPS_JAL | ((replacement >> 2) & UINT32_C(0x03FFFFFF));
    return 0;
}
