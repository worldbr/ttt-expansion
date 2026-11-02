#!/usr/bin/env python3
"""Prevent shipping a stale 3-character plugin or broken GitHub workflow."""
from pathlib import Path
import re
r=Path(__file__).resolve().parents[1]
main=(r/'src/main.c').read_text()
menu=(r/'src/menu_state.c').read_text()
workflow=(r/'.github/workflows/compilar-prx.yml').read_text()
make=(r/'Makefile').read_text()
assert '[TTT MENU47 V0.4.3] module_start OK' in main
assert 'native_v04_check(menu.character,0)' in main
assert 'ttt_native_v04_repair_all' in main
assert 'ttt_menu_slot_supported(unsigned slot){return slot<TTT_CHARACTER_COUNT;}' in menu
assert '#define TTT_CHARACTER_COUNT 47u' in (r/'src/character_ids.h').read_text()
assert 'src/native_guard.o' in make and 'src/match_cycle.o' in make
assert 'cp LEIA_PRIMEIRO_V043.md dist/LEIA_PRIMEIRO_V043.md' in workflow
assert 'cp LEIA_PRIMEIRO_REBUILD_V2.md' not in workflow
assert (r/'LEIA_PRIMEIRO_V043.md').is_file()
assert (r/'TTT_EXPANSION/characters.txt').is_file()
assert (r/'TTT_EXPANSION/plugin.ini').is_file()
assert len([x for x in (r/'TTT_EXPANSION/characters.txt').read_text().splitlines() if re.match(r'^\d\d=',x)])==47
print('PASS: V0.4.3 build contract, 47 characters, all required distribution files')
