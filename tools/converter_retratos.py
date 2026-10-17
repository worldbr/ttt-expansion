#!/usr/bin/env python3
"""Converte PNG/JPG para RGBA8888 64x64 (16.384 bytes), lido pelo plugin PRX.
Uso: py -m pip install pillow
     py tools/converter_retratos.py MEU_GOKU.png TTT_EXPANSION/portraits/goku_early.rgba
"""
import sys
from pathlib import Path
try:
    from PIL import Image, ImageOps
except ImportError:
    sys.exit("Instale Pillow primeiro: py -m pip install pillow")
if len(sys.argv)!=3: sys.exit("Uso: python tools/converter_retratos.py entrada.png saida.rgba")
image=Image.open(sys.argv[1]).convert('RGBA')
image=ImageOps.fit(image,(64,64),Image.Resampling.LANCZOS)
output=Path(sys.argv[2]);output.parent.mkdir(parents=True,exist_ok=True)
output.write_bytes(image.tobytes('raw','RGBA'))
print(f"Gravado: {output} ({output.stat().st_size} bytes)")
