#!/usr/bin/env python3
"""Patch candidato SOMENTE para o EBOOT.OLD ELF ULUS10537 analisado.
Verifica o opcode MIPS original no offset exato antes de salvar NOVA copia.
"""
import argparse
from pathlib import Path
OFFSET=0x2CBD4
VALID=(bytes.fromhex('11 37 1E 34'),bytes.fromhex('12 37 1E 34'),bytes.fromhex('34 37 1E 34'))
NEW=VALID[-1]
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--input',required=True);p.add_argument('--output',required=True)
    a=p.parse_args(); inp=Path(a.input).resolve();out=Path(a.output).resolve()
    if inp==out or out.exists():p.error('Saida precisa ser um arquivo novo; backups obrigatorios')
    data=bytearray(inp.read_bytes())
    if len(data)<OFFSET+4 or data[OFFSET:OFFSET+4] not in VALID:
        p.error('Opcode/offset nao corresponde ao EBOOT.OLD analisado. NAO aplique este patch a ISO/EBOOT.BIN desconhecido.')
    print(f'Opcode anterior: {data[OFFSET:OFFSET+4].hex(" ").upper()}')
    data[OFFSET:OFFSET+4]=NEW
    out.write_bytes(data)
    print(f'OK EBOOT.OLD: {out}, instruction 0x341E3734 / contador 0x3734')
if __name__=='__main__':main()
