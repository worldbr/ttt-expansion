#include "toggle.h"
void ttt_toggle_init(TTTToggle *t) { t->previous = 0; t->open = 0; }
int ttt_toggle_frame(TTTToggle *t, uint32_t buttons) {
    /* Opening/closing requires a NEW SQUARE press with both triggers held.
     * Holding Square and then browsing 1P-4P with L/R must NOT hide the menu.
     * Works if all three are pressed simultaneously or L+R then Square. */
    int square_rising = (buttons & TTT_SQUARE) && !(t->previous & TTT_SQUARE);
    int triggers_held = (buttons & (TTT_LTRIGGER|TTT_RTRIGGER)) == (TTT_LTRIGGER|TTT_RTRIGGER);
    t->previous = buttons;
    if(square_rising && triggers_held) { t->open = !t->open; return 1; }
    return 0;
}
