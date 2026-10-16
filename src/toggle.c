#include "toggle.h"
void ttt_toggle_init(TTTToggle *t) { t->previous = 0; t->open = 0; }
int ttt_toggle_frame(TTTToggle *t, uint32_t buttons) {
    int now = (buttons & TTT_HOTKEY) == TTT_HOTKEY;
    int before = (t->previous & TTT_HOTKEY) == TTT_HOTKEY;
    t->previous = buttons;
    if (now && !before) { t->open = !t->open; return 1; }
    return 0;
}
