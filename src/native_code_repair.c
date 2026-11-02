#include "native_code_repair.h"
#include "native_guard.h"
/* Byte-exact copy of the V0.4 ELF resolver opcodes checked by the V0.4
 * generator. Do not loosen this comparison: the game may overlay this
 * region with a DIFFERENT module while returning from a fight. */
static const uint32_t checked_expected[TTT_V04_CHECKED_WORDS]={
    0x2C8800A1u,0x15000007u,0x3C0308A7u,0x2482FF5Fu,
    0x00021040u,0x3463F294u,0x00621821u,0x03E00008u,
    0x84620000u,0x246343E0u,0x00641821u,0x90620000u,
    0x2C480080u,0x15000027u,0x2C4800AFu,0x11000025u,
    0x2442FF80u,0x00021880u,0x00431021u,0x00451021u,
    0x24423711u,0x03E00008u,0x7C021620u,0x00000000u
};
static const uint32_t unchecked_expected[TTT_V04_UNCHECKED_WORDS]={
    0x2C8800A1u,0x11000003u,0x3C0308A7u,0x0A271B31u,
    0x00000000u,0x3C0308A7u,0x24639720u,0x00641821u,
    0x90620000u,0x00021880u,0x00431021u,0x00451021u,
    0x24420593u,0x03E00008u,0x7C021620u
};
int ttt_v04_code_diagnose(const volatile uint32_t *checked,
                           const volatile uint32_t *unchecked,
                           uint32_t pack_count,
                           unsigned *module,unsigned *index,
                           uint32_t *actual,uint32_t *expected){
    unsigned i;
    if(module)*module=2u;
    if(index)*index=0u;
    if(actual)*actual=0;
    if(expected)*expected=0;
    if(!checked||!unchecked)return -1;
    if(pack_count!=TTT_V04_COUNT)return -2;
    for(i=0;i<TTT_V04_UNCHECKED_WORDS;i++){
        if(unchecked[i]!=unchecked_expected[i]){
            if(module)*module=1;
            if(index)*index=i;
            if(actual)*actual=unchecked[i];
            if(expected)*expected=unchecked_expected[i];
            return -1;
        }
    }
    for(i=1;i<TTT_V04_CHECKED_WORDS;i++){
        if(checked[i]!=checked_expected[i]){
            if(module)*module=0;
            if(index)*index=i;
            if(actual)*actual=checked[i];
            if(expected)*expected=checked_expected[i];
            return -1;
        }
    }
    if(checked[0]==checked_expected[0])return 0;
    if(module)*module=0;
    if(actual)*actual=checked[0];
    if(expected)*expected=checked_expected[0];
    return 1;
}
int ttt_v04_first_word_repairable(const volatile uint32_t *checked,
                                  const volatile uint32_t *unchecked,
                                  uint32_t pack_count){
    return ttt_v04_code_diagnose(checked,unchecked,pack_count,0,0,0,0);
}
