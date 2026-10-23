#!/usr/bin/env python3
"""Patch do contador SOMENTE no EBOOT descriptografado da variante testada ULUS10537.
A assinatura do ELF e do opcode sao verificadas. Nunca altera a origem.
"""
from pathlib import Path
import argparse
OFF=0x2CBD4
OLD=(bytes.fromhex('11 37 1E 34'),bytes.fromhex('12 37 1E 34'))
NEW=bytes.fromhex('34 37 1E 34')
def patch(src,out):
    src=Path(src).resolve();out=Path(out).resolve()
    if src==out or out.exists():raise ValueError('Destino deve ser novo e diferente da origem')
    b=bytearray(src.read_bytes())
    if len(b)<OFF+4 or b[:4]!=b'\x7fELF':raise ValueError('Requer EBOOT descriptografado ELF MIPS, NAO ~PSP/Prometheus')
    old=b[OFF:OFF+4]
    if old!=NEW and old not in OLD:raise ValueError('EBOOT nao corresponde ao opcode conhecido, abortando')
    b[OFF:OFF+4]=NEW
    out.write_bytes(b)
    print('OK: '+str(out)+' | contador 0x3734, EBOOT ELF validado')
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--eboot',required=True);p.add_argument('--output',required=True)
    a=p.parse_args();patch(a.eboot,a.output)
if __name__=='__main__':main()
