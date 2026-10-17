#!/usr/bin/env python3
"""Converte PNG para retrato RAW RGBA8888 64 ou 128 px, sem cabecalho.
Uso: py -m pip install pillow
     py tools/converter_retratos.py imagem.png TTT_EXPANSION/portraits/goku_early.rgba 128
"""
import sys
from pathlib import Path
from PIL import Image, ImageOps
if len(sys.argv) not in (3,4):
    sys.exit("Uso: python tools/converter_retratos.py entrada.png saida.rgba [64|128]")
size = int(sys.argv[3]) if len(sys.argv)==4 else 128
if size not in (64,128):sys.exit("Resolucao permitida: 64 ou 128")
img=Image.open(sys.argv[1]).convert('RGBA')
img=ImageOps.fit(img,(size,size),method=Image.Resampling.LANCZOS)
out=Path(sys.argv[2]);out.parent.mkdir(parents=True,exist_ok=True)
out.write_bytes(img.tobytes('raw','RGBA'))
print(f"OK: {out} | {size}x{size} RGBA8888 | {out.stat().st_size} bytes")
