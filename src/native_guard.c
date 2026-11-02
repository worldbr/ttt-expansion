#include "native_guard.h"
#include "character_ids.h"
#include <stddef.h>

static int check_code(TTTNativeGuard *st){
    if(st->checked0 != TTT_V04_CHECKED0 ||
       st->checked1 != TTT_V04_CHECKED1 ||
       st->unchecked0 != TTT_V04_UNCHECKED0 ||
       st->unchecked1 != TTT_V04_UNCHECKED1)
        return st->error = TTT_NATIVE_BAD_CODE;
    if(st->count != TTT_V04_COUNT)
        return st->error = TTT_NATIVE_BAD_COUNT;
    return TTT_NATIVE_VALID;
}
static void clear_status(TTTNativeGuard *st){
    st->error=TTT_NATIVE_VALID;
    st->bad_slot=TTT_CHARACTER_COUNT;
    st->found_text=st->found_group=st->expected_text=0;
    st->repaired_text=st->repaired_group=st->conflicts=0;
}
static int check_slot(TTTNativeGuard *st,
                      volatile uint8_t *txt,
                      volatile uint8_t *grp, unsigned slot){
    uint8_t id=ttt_character_battle_id(slot);
    uint8_t want=(uint8_t)(0x80u+slot);
    uint8_t found_txt=txt[id], found_grp=grp[id];
    if((found_txt!=want && found_txt!=0xFFu) ||
       (found_grp!=0u && found_grp!=0xFFu)){
        st->bad_slot=slot;
        st->found_text=found_txt;
        st->found_group=found_grp;
        st->expected_text=want;
        return TTT_NATIVE_CONFLICT;
    }
    return TTT_NATIVE_VALID;
}
static int repair_slot(TTTNativeGuard *st,
                       volatile uint8_t *txt,
                       volatile uint8_t *grp, unsigned slot){
    uint8_t id=ttt_character_battle_id(slot);
    uint8_t want=(uint8_t)(0x80u+slot);
    if(txt[id]==0xFFu){txt[id]=want;++st->repaired_text;}
    if(grp[id]==0xFFu){grp[id]=0u;++st->repaired_group;}
    if(txt[id]!=want || grp[id]!=0u){
        st->bad_slot=slot;
        st->found_text=txt[id];
        st->found_group=grp[id];
        st->expected_text=want;
        return TTT_NATIVE_VERIFY_FAILED;
    }
    return TTT_NATIVE_VALID;
}
int ttt_native_v04_ensure_slot(TTTNativeGuard *st,
                               volatile uint8_t *text_table,
                               volatile uint8_t *group_table,
                               unsigned slot){
    int rc;
    if(!st||!text_table||!group_table)return TTT_NATIVE_CONFLICT;
    clear_status(st);
    if(slot>=TTT_CHARACTER_COUNT)return st->error=TTT_NATIVE_BAD_SLOT;
    rc=check_code(st);
    if(rc!=TTT_NATIVE_VALID)return rc;
    rc=check_slot(st,text_table,group_table,slot);
    if(rc!=TTT_NATIVE_VALID){st->conflicts=1;return st->error=rc;}
    return st->error=repair_slot(st,text_table,group_table,slot);
}
/* Bulk audit is intentionally independent per slot. If another character
 * was modified by the game, it does not prevent a valid character from
 * being repaired and selected. We NEVER overwrite an unexpected byte. */
int ttt_native_v04_repair_all(TTTNativeGuard *st,
                              volatile uint8_t *text_table,
                              volatile uint8_t *group_table){
    unsigned slot, first_bad=TTT_CHARACTER_COUNT;
    uint8_t first_txt=0,first_grp=0,first_want=0;
    int rc;
    if(!st||!text_table||!group_table)return TTT_NATIVE_CONFLICT;
    clear_status(st);
    rc=check_code(st);
    if(rc!=TTT_NATIVE_VALID)return rc;
    for(slot=0;slot<TTT_CHARACTER_COUNT;slot++){
        rc=check_slot(st,text_table,group_table,slot);
        if(rc!=TTT_NATIVE_VALID){
            ++st->conflicts;
            /* Keep the first conflict for a useful diagnostic. */
            if(st->conflicts==1u){
                first_bad=slot;
                first_txt=st->found_text;
                first_grp=st->found_group;
                first_want=st->expected_text;
            }
            continue;
        }
        rc=repair_slot(st,text_table,group_table,slot);
        if(rc!=TTT_NATIVE_VALID)return st->error=rc;
    }
    if(st->conflicts){
        st->bad_slot=first_bad;
        st->found_text=first_txt;
        st->found_group=first_grp;
        st->expected_text=first_want;
    }
    return st->error=st->conflicts?TTT_NATIVE_PARTIAL:TTT_NATIVE_VALID;
}
/* Legacy all-or-nothing interface preserved for older callers/tests.
 * Validation of ALL 47 precedes any writes. */
int ttt_native_v04_ensure(TTTNativeGuard *st,
                          volatile uint8_t *text_table,
                          volatile uint8_t *group_table){
    unsigned slot;int rc;
    if(!st||!text_table||!group_table)return TTT_NATIVE_CONFLICT;
    clear_status(st);
    rc=check_code(st);
    if(rc!=TTT_NATIVE_VALID)return rc;
    for(slot=0;slot<TTT_CHARACTER_COUNT;slot++){
        rc=check_slot(st,text_table,group_table,slot);
        if(rc!=TTT_NATIVE_VALID){st->conflicts=1;return st->error=rc;}
    }
    for(slot=0;slot<TTT_CHARACTER_COUNT;slot++){
        rc=repair_slot(st,text_table,group_table,slot);
        if(rc!=TTT_NATIVE_VALID)return st->error=rc;
    }
    return TTT_NATIVE_VALID;
}
