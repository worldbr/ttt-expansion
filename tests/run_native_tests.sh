#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
CFLAGS=(-std=gnu11 -Wall -Wextra -Werror -Isrc)
run_test() {
  local name="$1"; shift
  gcc "${CFLAGS[@]}" "tests/$name.c" "$@" -o "/tmp/ttt_expansion_${name}_$$"
  "/tmp/ttt_expansion_${name}_$$"
  rm -f "/tmp/ttt_expansion_${name}_$$"
}
python3 tests/test_packed_skin.py
run_test test_ui_text src/ui_text.c
run_test test_dlc_story src/dlc_story.c
run_test test_gu_budget
run_test test_native src/overlay.c src/toggle.c src/framecheck.c
run_test test_menu_state src/menu_state.c
run_test test_bridge src/battle_bridge.c
run_test test_resource src/resource_hook.c
run_test test_actor_watch src/actor_watch.c
run_test test_slot_plan src/slot_plan.c
run_test test_callsite_plan src/callsite_plan.c
run_test test_input_guard src/input_guard.c src/input_plan.c

run_test test_actor_inject src/actor_inject.c

# PSP skin loader simulated end-to-end using host file IO and user memory API.
SKIN_TEST="/tmp/ttt_skin_loader_mock_$$"
gcc -DTTT_BRIDGE_PSP -std=gnu11 -O2 -Wall -Wextra -Werror -ffunction-sections -fdata-sections \
  -Itests/mock_psp_sdk -Isrc tests/test_skin_loader_mock.c src/gpu_probe.c src/ui_text.c \
  -Wl,--gc-sections -o "$SKIN_TEST"
"$SKIN_TEST" TTT_EXPANSION 0 3
"$SKIN_TEST" /tmp/ttt_no_such_assets_$$ 0 0
"$SKIN_TEST" TTT_EXPANSION 1 0
rm -f "$SKIN_TEST"
