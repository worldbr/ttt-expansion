#include <assert.h>
#include <stdio.h>
#include "dlc_story.h"
int main(void){
    unsigned i,id,color;uint32_t done=0;
    assert(ttt_dlc_mission(6u)==0);
    for(i=0;i<TTT_DLC_MISSIONS;i++){
        const TTTDlcMission *m=ttt_dlc_mission(i);
        assert(m&&m->title&&m->chapter&&m->goal);
        assert(ttt_dlc_mission_choices(i,&id,&color)==0);
        assert(id<3u&&color<4u);
        done=ttt_dlc_progress_mark(done,i);
    }
    assert(done==0x3fu);
    assert(ttt_dlc_progress_mark(done,99)==done);
    assert(ttt_dlc_progress_sanitize(0xffffffffu)==0x3fu);
    assert(ttt_dlc_mission_choices(7,&id,&color)<0);
    puts("OK DLC missions, progress and id/color mappings");
    return 0;
}
