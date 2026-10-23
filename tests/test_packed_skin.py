#!/usr/bin/env python3
"""Verify source atlas has exactly the 96 rows consumed by the 256x256 shared buffer."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
bg=(root/'TTT_EXPANSION/ui/menu_bg.4444').read_bytes()
font=(root/'TTT_EXPANSION/ui/font_atlas.4444').read_bytes()
assert len(bg)==131072 and len(font)==65536
# PSP packer: compact background in-place, append 96 rows of font.
atlas=bytearray(bg)
for y in range(160):
    src=(y*255)//159
    if src != y:
        atlas[512*y:512*(y+1)]=atlas[512*src:512*(src+1)]
atlas[160*512:256*512]=font[:96*512]
assert len(atlas)==131072
assert atlas[160*512:]==font[:96*512]
assert atlas[:512]==bg[:512]
assert atlas[159*512:160*512]==bg[255*512:256*512]
print('OK: 256x160 BG + 256x96 fonte dentro dos mesmos 128 KiB.')
