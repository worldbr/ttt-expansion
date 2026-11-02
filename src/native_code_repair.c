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

/* V0.4.7: both resolver regions may be scene-overlaid after a battle.
 * In diagnostico(8) the second battle injected IDs 0x16/0x01/0x02,
 * but the NEXT menu reported phase=-3 because an instruction after the
 * first two words of the unchecked resolver changed. The native code is
 * never called or written to here. Return STAGED when a known post-battle
 * checked-region signature, intact branch sentinels, a previously observed
 * pristine V0.4 image and an actual completed actor injection all agree.
 * The caller separately validates the selected ID's existing table bytes.
 * Reject unknown prefixes, missing sentinels, bad count or missing history.
 */
int ttt_v04_resolver_phase(const volatile uint32_t *checked,
                           const volatile uint32_t *unchecked,
                           uint32_t count,int pristine_seen,int actor_hit_seen){
    if(!checked||!unchecked)return -1;
    if(count!=TTT_V04_COUNT)return -2;
    if(ttt_v04_code_diagnose(checked,unchecked,count,0,0,0,0)==0)return 0;
    if(!pristine_seen||!actor_hit_seen)return -4;
    /* Distinguishes read-only foreign-scene data from an unknown code patch.
     * The 0x682A/0x682B high-halfword patterns are observed in both logs. */
    if((checked[0]&UINT32_C(0xFFFE0000))!=UINT32_C(0x682A0000))return -5;
    if(checked[1]!=checked_expected[1])return -6;
    if(unchecked[0]!=unchecked_expected[0] ||
       unchecked[1]!=unchecked_expected[1])return -7;
    /* Remaining unchecked words can be owned by another scene: DO NOT use
     * them to veto a staged actor selection. No game code is patched here. */
    return 1;
}
