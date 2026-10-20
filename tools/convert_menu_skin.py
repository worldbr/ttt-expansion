#!/usr/bin/env python3
"""Convert editable HD PNG into ABGR4444 used by PSP GE, no PSP SDK required.
Usage: python tools/convert_menu_skin.py [--background PATH] [--font PATH]
Needs Pillow.  Does not need a Windows EXE.
"""
from PIL import Image
from pathlib import Path
import argparse

def pack_4444(im):
    result=bytearray(im.width*im.height*2)
    for i,(r,g,b,a) in enumerate(im.convert('RGBA').getdata()):
        n=((a>>4)<<12)|((b>>4)<<8)|((g>>4)<<4)|(r>>4)
        result[i*2]=n&255
        result[i*2+1]=n>>8
    return bytes(result)

def build(bg,font,ui):
    ui.mkdir(parents=True,exist_ok=True)
    im=Image.open(bg).convert('RGBA')
    down=im.resize((256,256),Image.Resampling.LANCZOS)
    (ui/'menu_bg.4444').write_bytes(pack_4444(down))
    f=Image.open(font).convert('RGBA')
    if f.size!=(256,128):raise ValueError('A fonte deve ser atlas 256x128, ASCII 32-127, celulas 16x16')
    (ui/'font_atlas.4444').write_bytes(pack_4444(f))
    assert (ui/'menu_bg.4444').stat().st_size == 131072
    assert (ui/'font_atlas.4444').stat().st_size == 65536
    print('Criadas as texturas PSP: menu_bg.4444 (128 KB), font_atlas.4444 (64 KB)')

if __name__=='__main__':
    ap=argparse.ArgumentParser()
    base=Path(__file__).resolve().parents[1]/'TTT_EXPANSION'/'ui'
    ap.add_argument('--background',type=Path,default=base/'menu_bg_HD.png')
    ap.add_argument('--font',type=Path,default=base/'font_atlas.png')
    ap.add_argument('--output',type=Path,default=base)
    a=ap.parse_args()
    build(a.background,a.font,a.output)
