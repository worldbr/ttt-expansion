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
assert 'skin_frame_fallback(bits)' in src
assert 'render_menu_legacy(bits)' not in src
assert 'Goku (Inicio)' in src
assert 'Goku (Meio)' in src
assert 'Super Saiyajin' in src
assert 'skin_load_all()' in src
assert 'sceKernelAllocPartitionMemory' in src
assert 'sceKernelGetBlockHeadAddr' in src
assert 'sceKernelFreePartitionMemory' in src
assert 'buf=(uint8_t*)memalign(' not in src
assert 'TTT_BG_PATH' in src and 'TTT_FONT_PATH' in src
assert 'TTT_MENU_BG_BYTES (256u*256u*2u)' in src
assert 'TTT_FONT_BYTES (256u*128u*2u)' in src
assert 'PORT_MAX_W 64' in src
assert 'ttt_skin_ready' in src
assert 'ttt_skin_mask' in src
assert 'm->focus=1' in (root/'src/menu_state.c').read_text()
assert '(uint32_t)(m->focus&1u)<<31' in (root/'src/menu_state.c').read_text()
assert 'ttt_res_index_for' in (root/'src/resource_hook.c').read_text()
print('OK: V1.0.2 skin real, nomes, foco, retratos e indices preservados')
