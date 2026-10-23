#!/usr/bin/env python3
"""Valida assets e a interface realmente ativa do Menu 47; sem SDK PSP."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
ui = root / 'TTT_EXPANSION/ui'

# Assets herdados da V2.0.3: mesmo atlas de 128 KiB com fonte embutida.
assert (ui / 'menu_bg.4444').stat().st_size == 256 * 256 * 2
assert (ui / 'font_atlas.4444').stat().st_size == 256 * 128 * 2
for name in ('menu_bg_HD.png', 'PREVIEW_MENU_REFEITO.png', 'font_atlas.png'):
    assert (ui / name).read_bytes()[:8] == bytes.fromhex('89504e470d0a1a0a')
for name in ('goku_early', 'goku_mid', 'goku_mid_ssj'):
    assert (root / f'TTT_EXPANSION/portraits/{name}.rgba').stat().st_size == 64 * 64 * 4

src = (root / 'src/gpu_probe.c').read_text()
menu = (root / 'src/menu_state.c').read_text()
header = (root / 'src/menu_state.h').read_text()
main = (root / 'src/main.c').read_text()
audio = (root / 'src/menu_audio.c').read_text()
makefile = (root / 'Makefile').read_text()

# O antigo teste esperava a chamada skin_frame_fallback(bits), da V2.0.3.
# A interface ativa da V47 desenha render_menu47 e usa texto fallback
# quando o atlas nao carregar; nao usa o antigo render_menu_textured.
assert 'static void render_menu47(uint32_t bits)' in src
assert 'static void render_menu(uint32_t bits){render_menu47(bits);}' in src
assert 'for(i=0;i<47;i++)' in src
assert 'int x=14+(i%8)*35,y=56+(i/8)*29;' in src
assert 'static void menu47_label(' in src
assert 'if(ttt_skin_ready)skin_text_compact(' in src
assert 'else txt(x,y,t,1,color);' in src
assert 'render_menu_legacy(bits)' not in src

# 47 slots no menu, navegacao para quatro jogadores e quatro Colors.
assert '#define TTT_MENU47_SLOTS 47' in header
assert 'ttt_menu_slot_supported(unsigned slot){return slot<3u;}' in menu
assert 'm->player+1u)%4u' in menu
assert 'm->color+1u)%4u' in menu
assert '((uint32_t)(m->focus & 1u)<<10)' in menu
assert '((uint32_t)(m->sound & 1u)<<15)' in menu
assert 'if(!native_v03_ready())' in main

# Carregamento de retratos, fallback de fonte e alocacao PSP preservados.
for needle in (
    'skin_load_all()', 'sceKernelAllocPartitionMemory',
    'sceKernelGetBlockHeadAddr', 'sceKernelFreePartitionMemory',
    'TTT_BG_PATH', 'TTT_FONT_PATH',
    'TTT_MENU_BG_BYTES (256u*256u*2u)',
    'TTT_FONT_BYTES (256u*128u*2u)',
    'PORT_MAX_W 64', 'ttt_skin_ready', 'ttt_skin_mask',
):
    assert needle in src, needle

# Audio opcional do Menu 47 integrado na compilacao PSP.
assert 'sceAudioChReserve(' in audio
assert 'sceAudioOutputBlocking(' in audio
assert 'src/menu_audio.o' in makefile
assert '-lpspaudio' in makefile
print('OK: Menu47 ativo, 47 slots, J1-J4, C1-C4, retratos, fallback e audio')
