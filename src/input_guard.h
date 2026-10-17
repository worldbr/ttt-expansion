#ifndef TTT_INPUT_GUARD_H
#define TTT_INPUT_GUARD_H
#include <stdint.h>
/* Pure host-testable button policy: the external menu always sees the RAW pad.
 * The game's buffer is muted only while menu is open or a closing chord/button
 * must be released. Quarantine prevents the closing button leaking to the game.
 */
#define TTT_INPUT_CHORD UINT32_C(0x00008300) /* L + R + Square */
typedef struct TTTInputGuard { uint32_t quarantine; } TTTInputGuard;
void ttt_input_guard_init(TTTInputGuard *g);
void ttt_input_guard_hold(TTTInputGuard *g);
/* returns 1 when game's sample must be neutralized */
int ttt_input_guard_mute(TTTInputGuard *g, uint32_t raw_buttons, int menu_open);
#endif
