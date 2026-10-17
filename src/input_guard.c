#include "input_guard.h"
void ttt_input_guard_init(TTTInputGuard *g) { g->quarantine=0; }
void ttt_input_guard_hold(TTTInputGuard *g) { g->quarantine=1; }
int ttt_input_guard_mute(TTTInputGuard *g, uint32_t raw, int open) {
    if(open || (raw & TTT_INPUT_CHORD)==TTT_INPUT_CHORD) {
        g->quarantine=1;
        return 1;
    }
    if(g->quarantine) {
        /* Release ALL buttons before restoring game control. */
        if(raw!=0) return 1;
        g->quarantine=0;
    }
    return 0;
}
