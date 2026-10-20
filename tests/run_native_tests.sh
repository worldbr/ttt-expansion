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
