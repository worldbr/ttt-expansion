#include "native_guard.h"
#include "character_ids.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static volatile uint8_t text_table[0xA1];
static volatile uint8_t group_table[0xA1];
static TTTNativeGuard st;
static void init(void){
    unsigned i;
    memset((void*)text_table,0xff,sizeof(text_table));
    memset((void*)group_table,0xff,sizeof(group_table));
    st.checked0=TTT_V04_CHECKED0;
    st.checked1=TTT_V04_CHECKED1;
    st.unchecked0=TTT_V04_UNCHECKED0;
    st.unchecked1=TTT_V04_UNCHECKED1;
    st.count=TTT_V04_COUNT;
    for(i=0;i<TTT_CHARACTER_COUNT;i++){
        unsigned id=ttt_character_battle_id(i);
        text_table[id]=(uint8_t)(0x80u+i);
        group_table[id]=0;
    }
    /* Native character 03 was previously occupied and stays untouched. */
    text_table[3]=0;
    group_table[3]=0;
}
int main(void){
    int i;
    init();
    assert(ttt_native_v04_ensure(&st,text_table,group_table)==0);
    assert(st.repaired_text==0 && st.repaired_group==0);
    /* Simulate a battle return restoring two unused IDs to FF. */
    text_table[ttt_character_battle_id(36)]=0xff;
    group_table[ttt_character_battle_id(36)]=0xff;
    text_table[ttt_character_battle_id(1)]=0xff;
    assert(ttt_native_v04_ensure(&st,text_table,group_table)==0);
    assert(st.repaired_text==2 && st.repaired_group==1);
    assert(text_table[3]==0 && group_table[3]==0);
    assert(ttt_native_v04_ensure(&st,text_table,group_table)==0);
    assert(st.repaired_text==0 && st.repaired_group==0);
    /* A foreign overwrite MUST NOT result in any writes. */
    text_table[ttt_character_battle_id(30)]=0xff;
    text_table[ttt_character_battle_id(36)]=0x65;
    assert(ttt_native_v04_ensure(&st,text_table,group_table)==TTT_NATIVE_CONFLICT);
    assert(st.bad_slot==36);
    assert(text_table[ttt_character_battle_id(30)]==0xff);
    /* Signature changed: never write table bytes. */
    text_table[ttt_character_battle_id(36)]=0xff;
    st.checked1=0;
    assert(ttt_native_v04_ensure(&st,text_table,group_table)==TTT_NATIVE_BAD_CODE);
    assert(text_table[ttt_character_battle_id(30)]==0xff);
    st.checked1=TTT_V04_CHECKED1;
    st.count=0x3734;
    assert(ttt_native_v04_ensure(&st,text_table,group_table)==TTT_NATIVE_BAD_COUNT);
    assert(text_table[ttt_character_battle_id(30)]==0xff);
    /* count restored and all 47 slots rechecked. */
    st.count=TTT_V04_COUNT;
    assert(ttt_native_v04_ensure(&st,text_table,group_table)==0);
    for(i=0;i<(int)TTT_CHARACTER_COUNT;i++){
        unsigned id=ttt_character_battle_id((unsigned)i);
        assert(text_table[id]==(uint8_t)(0x80u+i));
        assert(group_table[id]==0);
    }
    /* V0.4.3: one unrelated, foreign mapping cannot disable 46 other IDs. */
    init();
    text_table[ttt_character_battle_id(36)]=0x65; /* unexpected byte */
    text_table[ttt_character_battle_id(1)]=0xff; /* scene reload */
    group_table[ttt_character_battle_id(1)]=0xff;
    assert(ttt_native_v04_ensure_slot(&st,text_table,group_table,1)==TTT_NATIVE_VALID);
    assert(st.repaired_text==1 && st.repaired_group==1);
    assert(text_table[ttt_character_battle_id(1)]==0x81);
    assert(group_table[ttt_character_battle_id(1)]==0);
    assert(text_table[ttt_character_battle_id(36)]==0x65);
    assert(ttt_native_v04_ensure_slot(&st,text_table,group_table,36)==TTT_NATIVE_CONFLICT);
    assert(st.bad_slot==36 && st.conflicts==1);
    assert(text_table[ttt_character_battle_id(36)]==0x65);
    /* Other slots may be repaired safely while conflict is isolated. */
    text_table[ttt_character_battle_id(20)]=0xff;
    group_table[ttt_character_battle_id(20)]=0xff;
    assert(ttt_native_v04_repair_all(&st,text_table,group_table)==TTT_NATIVE_PARTIAL);
    assert(st.conflicts==1 && st.bad_slot==36);
    assert(st.repaired_text==1 && st.repaired_group==1);
    assert(text_table[ttt_character_battle_id(20)]==(uint8_t)(0x80+20));
    assert(group_table[ttt_character_battle_id(20)]==0);
    assert(text_table[ttt_character_battle_id(36)]==0x65);
    /* Unknown group mapping of SELECTED slot forbids even partial writes. */
    init();
    text_table[ttt_character_battle_id(3)]=0xff;
    group_table[ttt_character_battle_id(3)]=0x42;
    assert(ttt_native_v04_ensure_slot(&st,text_table,group_table,3)==TTT_NATIVE_CONFLICT);
    assert(text_table[ttt_character_battle_id(3)]==0xff);
    assert(group_table[ttt_character_battle_id(3)]==0x42);
    /* Verify code signature and count before any table modification. */
    init();
    text_table[ttt_character_battle_id(3)]=0xff;
    st.checked1=0;
    assert(ttt_native_v04_ensure_slot(&st,text_table,group_table,3)==TTT_NATIVE_BAD_CODE);
    assert(text_table[ttt_character_battle_id(3)]==0xff);
    st.checked1=TTT_V04_CHECKED1;
    st.count=0x3734;
    assert(ttt_native_v04_ensure_slot(&st,text_table,group_table,3)==TTT_NATIVE_BAD_COUNT);
    assert(text_table[ttt_character_battle_id(3)]==0xff);
    st.count=TTT_V04_COUNT;
    assert(ttt_native_v04_ensure_slot(&st,text_table,group_table,47)==TTT_NATIVE_BAD_SLOT);
    puts("PASS: V0.4.3 native guard 47 IDs, targeted second-round repair, partial conflict isolation, safe code checks");
    return 0;
}
