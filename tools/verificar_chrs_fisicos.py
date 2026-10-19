#!/usr/bin/env python3
"""Auditor somente leitura de PACKFILE resultante, sem dependencias externas."""
import argparse, hashlib, struct
from pathlib import Path
from expandir_packfile_7_gokus import DATA, EXPANDED, FIRST, ORIGINAL_CHR, entry_idx, decode_record

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--packfile',required=True)
    ap.add_argument('--asset',required=True)
    a=ap.parse_args()
    pack=Path(a.packfile);asset=Path(a.asset)
    if not pack.is_file() or not asset.is_file():ap.error('Arquivo nao encontrado')
    with pack.open('rb') as f:head=f.read(DATA)
    count=struct.unpack_from('<I',head,4)[0]
    assert count==EXPANDED,f'Contagem invalida: {count:#x}'
    first_off,first_size=decode_record(head,FIRST)
    old_off,old_size=decode_record(head,ORIGINAL_CHR)
    assert first_off>old_off,'O novo CHR nao esta apos o antigo'
    assert first_off+first_size<=pack.stat().st_size,'O CHR extrapola o PACKFILE'
    assert first_size==asset.stat().st_size,'Tamanho no indice difere do asset'
    h1=hashlib.sha256();h2=hashlib.sha256()
    with pack.open('rb') as f:
        f.seek(first_off)
        remaining=first_size
        while remaining:
            d=f.read(min(1024*1024,remaining));assert d
            h1.update(d);remaining-=len(d)
    with asset.open('rb') as f:
        for d in iter(lambda:f.read(1024*1024),b''):h2.update(d)
    assert h1.digest()==h2.digest(),'Bytes do CHR novo nao coincidem com o asset original'
    print('OK: PACKFILE 0x3734, novo CHR fisico e SHA-256 identico')
    print(f'Entrada 14098: offset {first_off:#x}, tamanho {first_size:#x}')
    print(f'CHR original:  offset {old_off:#x}, tamanho {old_size:#x}')
    print(f'SHA-256 {h1.hexdigest()}')
if __name__=='__main__':main()
