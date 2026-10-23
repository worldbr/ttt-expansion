#ifndef TTT_DLC_STORY_H
#define TTT_DLC_STORY_H
#include <stdint.h>
#define TTT_DLC_MISSIONS 6u
typedef struct TTTDlcMission {
    const char *title;
    const char *chapter;
    const char *goal;
    uint8_t character;
    uint8_t color;
} TTTDlcMission;
const TTTDlcMission *ttt_dlc_mission(unsigned index);
uint32_t ttt_dlc_progress_sanitize(uint32_t flags);
uint32_t ttt_dlc_progress_mark(uint32_t flags,unsigned index);
int ttt_dlc_mission_choices(unsigned index, unsigned *id, unsigned *color);
#endif
