#!/usr/bin/env python3
"""Validador SOMENTE LEITURA do EBOOT V0.4 e header PACKFILE 47.

Uso: python tools/verificar_instalacao_v043.py --elf EBOOT.ELF --header CABECALHO.BIN
Nao modifica ISO, ELF ou PACKFILE. Nao garante funcionamento em batalha.
"""
import argparse
import hashlib
import struct
from pathlib import Path
from gerar_eboot_packfile_v04 import (
    IDS, M, CHECKED, UNCHECKED, TEXT_MAP, GROUP_MAP, COUNT_ADDR,
    NEW_COUNT, FIRST, SLOT_CODE_BASE,
)
from inspecionar_packfile import DATA, decode


def check_elf(path):
    p = Path(path)
    e = p.read_bytes()
    errors = []
    if len(e) != 3747908 or e[:4] != b'\x7fELF':
        errors.append('ELF diferente do layout EBOOT.OLD V0.3/V0.4 analisado (tamanho ou magic)')
        return errors
    for addr, want, label in (
        (CHECKED, 0x2C8800A1, 'CHECKED[0]'),
        (CHECKED + 4, 0x15000007, 'CHECKED[1]'),
        (UNCHECKED, 0x2C8800A1, 'UNCHECKED[0]'),
        (UNCHECKED + 4, 0x11000003, 'UNCHECKED[1]'),
        (COUNT_ADDR, 0x341E37FC, 'contador EBOOT 0x37FC'),
    ):
        found = struct.unpack_from('<I', e, M(addr))[0]
        if found != want:
            errors.append(f'{label}: esperado 0x{want:08X}, encontrado 0x{found:08X}')
    for slot, bid in enumerate(IDS):
        txt = e[M(TEXT_MAP) + bid]
        grp = e[M(GROUP_MAP) + bid]
        want = SLOT_CODE_BASE + slot
        if txt != want or grp != 0:
            errors.append(f'Slot {slot+1:02d} ID {bid:02X}: esperado text={want:02X} group=00; encontrado {txt:02X}/{grp:02X}')
    print(f'EBOOT: {p.name} | SHA256 {hashlib.sha256(e).hexdigest()}')
    print(f'  {len(IDS)} IDs mapeados | problemas: {len(errors)}')
    return errors


def check_header(path):
    p = Path(path)
    with p.open('rb') as f:
        h = f.read(DATA)
    errors = []
    if len(h) != DATA or h[:4] != b'PacK':
        return ['Header PACKFILE invalido ou incompleto (esperado 0x38000 bytes, magic PacK)']
    count = struct.unpack_from('<I', h, 4)[0]
    if count != NEW_COUNT:
        errors.append(f'Contador PACKFILE: esperado 0x{NEW_COUNT:04X}, encontrado 0x{count:04X}')
    for slot in range(47):
        for color_or_sfx in range(5):
            idx = FIRST + 5*slot + color_or_sfx
            pair = decode(h, idx)
            if pair is None or pair[0] < DATA or pair[1] <= 0:
                errors.append(f'Slot {slot+1:02d} recurso {color_or_sfx+1}: indice 0x{idx:04X} ausente/invalido')
    print(f'PACKFILE: {p.name} | contador 0x{count:04X} | {47*5} indices | problemas: {len(errors)}')
    return errors


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--elf', required=True, help='EBOOT ELF realmente instalado na ISO')
    ap.add_argument('--header', help='PACKFILE.BIN ou header extraido 0x38000 bytes')
    args = ap.parse_args()
    errors = check_elf(args.elf)
    if args.header:
        errors.extend(check_header(args.header))
    if errors:
        print('FALHOU: EBOOT/header nao conferem com o projeto de 47 IDs:')
        for error in errors[:12]:
            print(' -', error)
        if len(errors) > 12:
            print(' ... e mais', len(errors)-12, 'problemas')
        raise SystemExit(1)
    print('OK: estrutura V0.4 dos 47 IDs verificada no disco. Teste real PPSSPP ainda necessario.')


if __name__ == '__main__':
    main()
