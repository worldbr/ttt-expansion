#!/usr/bin/env python3
"""Checks shipped assets and menu C integration, no PSP SDK needed."""
from pathlib import Path
import struct
root=Path(__file__).resolve().parents[1]
ui=root/'TTT_EXPANSION/ui'
assert (ui/'menu_bg.4444').stat().st_size == 256*256*2
assert (ui/'font_atlas.4444').stat().st_size == 256*128*2
for f in ['menu_bg_HD.png','PREVIEW_MENU_REFEITO.png','font_atlas.png']:
    assert (ui/f).read_bytes()[:8] == bytes.fromhex('89504e470d0a1a0a')
for name in ['goku_early','goku_mid','goku_mid_ssj']:
    assert (root/f'TTT_EXPANSION/portraits/{name}.rgba').stat().st_size == 64*64*4
src=(root/'src/gpu_probe.c').read_text()
assert 'render_menu_textured' in src
assert 'skin_load_all()' in src
assert 'memalign(16' in src
assert 'TTT_MENU_BG_BYTES (256u*256u*2u)' in src
assert 'TTT_FONT_BYTES (256u*128u*2u)' in src
assert 'PORT_MAX_W 64' in src
assert 'ttt_skin_ready' in src
assert 'ttt_res_index_for' in (root/'src/resource_hook.c').read_text()
print('OK: skin assets, 3 retratos, renderer PSP e indices preservados')
