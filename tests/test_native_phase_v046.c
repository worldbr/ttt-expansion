#include "native_code_repair.h"
#include "native_guard.h"
#include "character_ids.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
static uint32_t checked[24]={
 0x2C8800A1u,0x15000007u,0x3C0308A7u,0x2482FF5Fu,0x00021040u,
 0x3463F294u,0x00621821u,0x03E00008u,0x84620000u,0x246343E0u,
 0x00641821u,0x90620000u,0x2C480080u,0x15000027u,0x2C4800AFu,
 0x11000025u,0x2442FF80u,0x00021880u,0x00431021u,0x00451021u,
 0x24423711u,0x03E00008u,0x7C021620u,0x00000000u};
static uint32_t unchecked[15]={
 0x2C8800A1u,0x11000003u,0x3C0308A7u,0x0A271B31u,0x00000000u,
 0x3C0308A7u,0x24639720u,0x00641821u,0x90620000u,0x00021880u,
 0x00431021u,0x00451021u,0x24420593u,0x03E00008u,0x7C021620u};
int main(void){
 uint8_t txt[0xA1],grp[0xA1]; TTTNativeGuard st={0};unsigned i;uint8_t id;
 memset(txt,255,sizeof(txt));memset(grp,255,sizeof(grp));
 for(i=0;i<TTT_CHARACTER_COUNT;i++){
  id=ttt_character_battle_id(i);txt[id]=(uint8_t)(0x80+i);grp[id]=0;
 }
 st.count=0x37FC;
 assert(ttt_v04_resolver_phase(checked,unchecked,0x37FC,0,0)==0);
 checked[0]=0x682B5B6Au; checked[5]=0x682B6E8Au;
 assert(ttt_v04_resolver_phase(checked,unchecked,0x37FC,0,1)==-4);
 assert(ttt_v04_resolver_phase(checked,unchecked,0x37FC,1,0)==-4);
 assert(ttt_v04_resolver_phase(checked,unchecked,0x37FC,1,1)==1);
 checked[0]=0x682A78BAu;checked[5]=0x682A8BE2u;
 assert(ttt_v04_resolver_phase(checked,unchecked,0x37FC,1,1)==1);
 assert(ttt_native_v04_verify_mappings(&st,txt,grp,1,0)==0);
 /* Read-only: a missing marker is NOT repaired while another scene owns code. */
 id=ttt_character_battle_id(1);txt[id]=0xff;
 assert(ttt_native_v04_verify_mappings(&st,txt,grp,1,0)==TTT_NATIVE_CONFLICT);
 assert(txt[id]==0xff && grp[id]==0);
 txt[id]=0x81;
 assert(ttt_native_v04_verify_mappings(&st,txt,grp,1,0)==0);
 /* Third fight: unchecked word 5 was overwritten by the scene, but
  * checked[0], checked[1], unchecked[0:2] still show expected sentinels.
  * Previously this returned -3 and permanently rejected character X. */
 unchecked[5]^=1u;
 assert(ttt_v04_resolver_phase(checked,unchecked,0x37FC,1,1)==1);
 assert(ttt_native_v04_verify_mappings(&st,txt,grp,1,0)==0);
 assert(unchecked[5]==(0x3C0308A7u^1u)); /* no code mutation */
 /* Repeat multiple rounds: the overlay region can change on each fight.
  * Staging remains read-only, while unknown signatures are still refused. */
 for(int round=2;round<=5;round++){
  uint32_t before=(round&1)?0x682A1111u:0x682B2222u;
  checked[0]=before;
  unchecked[8]=0x682A3000u+(uint32_t)round;
  assert(ttt_v04_resolver_phase(checked,unchecked,0x37FC,1,1)==1);
  assert(checked[0]==before);
  assert(ttt_native_v04_verify_mappings(&st,txt,grp,1,0)==0);
 }
 unchecked[8]=0x90620000u;
 unchecked[0]=0x12345678u;
 assert(ttt_v04_resolver_phase(checked,unchecked,0x37FC,1,1)==-7);
 unchecked[0]=0x2C8800A1u;
 unchecked[1]=0x12345678u;
 assert(ttt_v04_resolver_phase(checked,unchecked,0x37FC,1,1)==-7);
 unchecked[1]=0x11000003u;
 checked[1]=0x11111111u;
 assert(ttt_v04_resolver_phase(checked,unchecked,0x37FC,1,1)==-6);
 checked[1]=0x15000007u;
 unchecked[5]^=1u;
 assert(ttt_v04_resolver_phase(checked,unchecked,0x3734,1,1)==-2);
 checked[0]=0x12345678u;
 assert(ttt_v04_resolver_phase(checked,unchecked,0x37FC,1,1)==-5);
 puts("PASS V0.4.7: segunda/terceira escolhas diferidas, sem escrita em codigo de outra cena");
 return 0;
}
