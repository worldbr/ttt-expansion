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

uint32_t ttt_v04_expected_checked_word(unsigned index){
    return index<TTT_V04_CHECKED_WORDS?checked_expected[index]:0u;
}
/* Pure/read-only decision. Return 1 only for a small, observed post-battle
 * overwrite pattern in two known indices. 0 = intact; negative = refuse. */
int ttt_v04_repair_plan(const volatile uint32_t *checked,
                        const volatile uint32_t *unchecked,
                        uint32_t pack_count,int pristine_seen,
                        uint32_t *mask){
    uint32_t diff=0u;unsigned i;
    if(mask)*mask=0u;
    if(!checked || !unchecked || !mask)return -1;
    if(pack_count!=TTT_V04_COUNT)return -2;
    for(i=0;i<TTT_V04_UNCHECKED_WORDS;i++)
        if(unchecked[i]!=unchecked_expected[i])return -3;
    for(i=0;i<TTT_V04_CHECKED_WORDS;i++){
        const uint32_t actual=checked[i];
        if(actual==checked_expected[i])continue;
        /* Only these two indices exhibited the 0x682Bxxxx overwrite in
         * diagnostico(7). The branch, return and load logic stays intact. */
        if(i!=0u && i!=9u)return -5;
        if((actual&0xFFFF0000u)!=0x682B0000u)return -6;
        diff|=1u<<i;
    }
    if(!diff)return 0;
    if(!pristine_seen)return -4;
    *mask=diff;
    return 1;
}

/* V0.4.6: the checked resolver address is scene-overlaid after a battle.
 * Do NOT restore instructions at this address while the other scene owns it.
 * Returning 1 means only that a NEW selection may be staged, provided its
 * mapping bytes are already valid; it does NOT mean the resolver is callable.
 * 0: all bytes match V0.4; 1: deferred until next battle scene; negative: refuse.
 */
int ttt_v04_resolver_phase(const volatile uint32_t *checked,
                           const volatile uint32_t *unchecked,
                           uint32_t count,int pristine_seen,int actor_hit_seen){
    unsigned i;
    if(!checked||!unchecked)return -1;
    if(count!=TTT_V04_COUNT)return -2;
    if(ttt_v04_code_diagnose(checked,unchecked,count,0,0,0,0)==0)return 0;
    if(!pristine_seen||!actor_hit_seen)return -4;
    for(i=0;i<TTT_V04_UNCHECKED_WORDS;i++)
        if(unchecked[i]!=unchecked_expected[i])return -3;
    /* Both prefixes were recorded in diagnostico(8), on different fights.
     * 0x682Axxxx and 0x682Bxxxx are considered scene data, NOT code to run. */
    if((checked[0]&UINT32_C(0xFFFE0000))!=UINT32_C(0x682A0000))return -5;
    return 1;
}
