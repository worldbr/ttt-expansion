#ifndef TTT_UI_TEXT_H
#define TTT_UI_TEXT_H
#include <stdint.h>
#define TTT_UI_TEXT_LANGS 3
#define TTT_UI_TEXT_MAX 96
#define TTT_UI_TEXT_LINES_MAX 8192
/* Key names and all visible UI strings are customizable in the lang directory.
 * Main thread loads them before installing the GPU hook. GPU reads stable
 * static strings; never mutate them while a frame is being drawn. */
typedef enum {
    TTT_TEXT_TITLE,
    TTT_TEXT_TAB_CHARACTERS,
    TTT_TEXT_TAB_MISSIONS,
    TTT_TEXT_TAB_MAPS,
    TTT_TEXT_TAB_OPTIONS,
    TTT_TEXT_COMING_SOON,
    TTT_TEXT_EXTRA_CHARACTERS,
    TTT_TEXT_PREVIEW,
    TTT_TEXT_FUTURE,
    TTT_TEXT_STATUS_PREVIEW,
    TTT_TEXT_STATUS_CONFIRMED,
    TTT_TEXT_STATUS_ACTIVE,
    TTT_TEXT_CHAR0_NAME,
    TTT_TEXT_CHAR1_NAME,
    TTT_TEXT_CHAR2_NAME,
    TTT_TEXT_CHAR2_SUBTITLE,
    TTT_TEXT_CHAR0_CARD_TOP,
    TTT_TEXT_CHAR0_CARD_BOTTOM,
    TTT_TEXT_CHAR1_CARD_TOP,
    TTT_TEXT_CHAR1_CARD_BOTTOM,
    TTT_TEXT_CHAR2_CARD_TOP,
    TTT_TEXT_CHAR2_CARD_BOTTOM,
    TTT_TEXT_ID_LABEL,
    TTT_TEXT_PLAYER_LABEL,
    TTT_TEXT_COLOR_LABEL,
    TTT_TEXT_FOCUS_CHARACTERS,
    TTT_TEXT_FOCUS_COLORS,
    TTT_TEXT_MISSIONS_SOON_TITLE,
    TTT_TEXT_MISSIONS_SOON_DETAIL,
    TTT_TEXT_MAPS_SOON_TITLE,
    TTT_TEXT_MAPS_SOON_DETAIL,
    TTT_TEXT_OPTIONS_SOON_TITLE,
    TTT_TEXT_OPTIONS_SOON_DETAIL,
    TTT_TEXT_FOOTER_MOVE,
    TTT_TEXT_FOOTER_CONFIRM,
    TTT_TEXT_FOOTER_CLOSE,
    TTT_TEXT_TEXTURE_ERROR,
    TTT_TEXT_MISSION_RESERVED,
    TTT_TEXT_COUNT
} TTTTextKey;
typedef struct {
    int open_rc, bytes_read, overrides, invalid_lines;
} TTTTextFileStats;
void ttt_text_defaults(void);
int ttt_text_parse_line(unsigned lang, const char *line);
void ttt_text_load_external(void);
const char *ttt_text(unsigned lang, TTTTextKey key);
TTTTextFileStats ttt_text_stats(unsigned lang);
#endif
