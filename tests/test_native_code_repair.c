#include "native_code_repair.h"
#include "native_guard.h"
#include <assert.h>
#include <stdio.h>
static uint32_t checked[24]={
    0x2C8800A1u,0x15000007u,0x3C0308A7u,0x2482FF5Fu,
    0x00021040u,0x3463F294u,0x00621821u,0x03E00008u,
    0x84620000u,0x246343E0u,0x00641821u,0x90620000u,
    0x2C480080u,0x15000027u,0x2C4800AFu,0x11000025u,
    0x2442FF80u,0x00021880u,0x00431021u,0x00451021u,
    0x24423711u,0x03E00008u,0x7C021620u,0x00000000u
};
static uint32_t unchecked[15]={
    0x2C8800A1u,0x11000003u,0x3C0308A7u,0x0A271B31u,
    0x00000000u,0x3C0308A7u,0x24639720u,0x00641821u,
    0x90620000u,0x00021880u,0x00431021u,0x00451021u,
    0x24420593u,0x03E00008u,0x7C021620u
};
int main(void){
    unsigned i;
    assert(ttt_v04_first_word_repairable(checked,unchecked,0x37FC)==0);
    assert(ttt_v04_first_word_repairable(checked,unchecked,0x3734)==-2);
    {
        unsigned module,index;uint32_t actual,want;
        checked[13]^=1u;
        assert(ttt_v04_code_diagnose(checked,unchecked,0x37FC,&module,&index,&actual,&want)==-1);
        assert(module==0u && index==13u && actual!=want);
        checked[13]^=1u;
    }
    checked[0]=0x682BE5B6u; /* Observed AFTER battle in diagnostico(6). */
    assert(ttt_v04_first_word_repairable(checked,unchecked,0x37FC)==1);
    for(i=1;i<24;i++){
        uint32_t save=checked[i];checked[i]^=1u;
        assert(ttt_v04_first_word_repairable(checked,unchecked,0x37FC)==-1);
        checked[i]=save;
    }
    for(i=0;i<15;i++){
        uint32_t save=unchecked[i];unchecked[i]^=1u;
        assert(ttt_v04_first_word_repairable(checked,unchecked,0x37FC)==-1);
        unchecked[i]=save;
    }
    assert(ttt_v04_first_word_repairable(0,unchecked,0x37FC)==-1);
    checked[0]=0x2C8800A1u;
    assert(ttt_v04_first_word_repairable(checked,unchecked,0x37FC)==0);
    puts("PASS: after-battle first opcode repair gated by all 38 remaining opcodes and 0x37FC");
    return 0;
}
