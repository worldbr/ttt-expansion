#include "native_code_repair.h"
#include <assert.h>
#include <stdint.h>
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
    uint32_t mask=0xFFFFFFFFu;
    assert(ttt_v04_repair_plan(checked,unchecked,0x37FC,0,&mask)==0 && mask==0);
    checked[0]=0x682B470Eu; checked[9]=0x682B4776u; /* diagnostic (7) */
    assert(ttt_v04_repair_plan(checked,unchecked,0x37FC,0,&mask)==-4 && mask==0);
    assert(ttt_v04_repair_plan(checked,unchecked,0x37FC,1,&mask)==1);
    assert(mask==((1u<<0)|(1u<<9)));
    assert(ttt_v04_expected_checked_word(0)==0x2C8800A1u);
    assert(ttt_v04_expected_checked_word(9)==0x246343E0u);
    checked[0]=0x682BE5B6u; /* diagnostic (6) */
    assert(ttt_v04_repair_plan(checked,unchecked,0x37FC,1,&mask)==1);
    checked[7]=0x682B1234u;
    assert(ttt_v04_repair_plan(checked,unchecked,0x37FC,1,&mask)==-5 && mask==0);
    checked[7]=0x03E00008u;
    checked[9]=0x12345678u;
    assert(ttt_v04_repair_plan(checked,unchecked,0x37FC,1,&mask)==-6);
    checked[9]=0x682B4776u;
    unchecked[6]^=1u;
    assert(ttt_v04_repair_plan(checked,unchecked,0x37FC,1,&mask)==-3);
    unchecked[6]^=1u;
    assert(ttt_v04_repair_plan(checked,unchecked,0x3734,1,&mask)==-2);
    checked[0]=0x2C8800A1u;checked[9]=0x246343E0u;
    assert(ttt_v04_repair_plan(checked,unchecked,0x37FC,1,&mask)==0);
    puts("PASS V0.4.5: pristine gate + post-battle resolver words 0 and 9 whitelist");
    return 0;
}
