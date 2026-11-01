#!/usr/bin/env python3
"""TTT ULUS10537 V0.4 — 47 battle IDs, 47 grupos de recursos, 47 placeholders.

Entrada: EBOOT.OLD V0.3 (SHA conhecido) e cabecalho PACKFILE_CABEÇALHO2.BIN
         com contagem 0x3734.
Saida: ELF do mesmo tamanho + header 0x38000 (nao ISO completo).

O codigo mantém os dois resolvedores em seus proprios limites (0x60 / 0x3c).
Usa 0x08A743E0 como um marcador de slot 0x80..0xAE, e a tabela de grupos
0x08A69720=0 para personagens de placeholder. Outros IDs permanecem intactos.
EBOOT/packfile so para testes, nao validado em batalha com novos IDs.
"""
import csv, hashlib, json, struct, sys
from pathlib import Path

HERE=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(Path(__file__).resolve().parent))
from inspecionar_packfile import decode, DATA
from dlc_packfile_builder import encode

USED_HEX='03 04 05 06 0D 0E 0F 10 11 12 13 15 17 19 1A 1B 1C 1D 22 23 24 25 2C 2D 27 28 29 2A 38 2E 2F 31 32 35 34 49 4D 4F 50 55 52 53 54 56 57 59 5A 58 5B 5C 5D 5E 5F 64 65 66 67 68 6D 69 6A 6B 6C 6F 70 72 74 75 85 63'
USED={int(s,16) for s in USED_HEX.split()}
RESERVED_ALIASES={0x46,0x47}  # aliases internos ja encontrados anteriormente
IDS=tuple(i for i in range(0xA1) if i not in USED and i not in RESERVED_ALIASES)[:47]
assert len(USED)==70 and len(IDS)==47 and IDS[:3]==(0,1,2)
FIRST=0x3711
NEW_COUNT=FIRST+47*5 # 0x37FC
OLD_COUNT=0x3734
SLOT_CODE_BASE=0x80
ELF_BASE=0x08804040
ELF_OFF=0x1018
CHECKED=0x089C6CA0
UNCHECKED=0x089C6D60
TEXT_MAP=0x08A743E0
GROUP_MAP=0x08A69720
COUNT_ADDR=0x0882FBFC
EXPECTED_ELF_SHA='c3c41e1248bcc8c1e199c218ef4f9c4fdaadccb5c44035673a40d08e50749520'

# MIPS32 instructions.
def I(op,rs,rt,imm):return ((op&63)<<26)|((rs&31)<<21)|((rt&31)<<16)|(imm&65535)
def R(rs,rt,rd,sa,fn):return ((rs&31)<<21)|((rt&31)<<16)|((rd&31)<<11)|((sa&31)<<6)|(fn&63)
def B(op,rs,rt,source,dest):
    assert (dest-source-4)%4==0
    delta=(dest-source-4)//4
    assert -32768<=delta<=32767
    return I(op,rs,rt,delta)
def J(to):return 0x08000000|((to>>2)&0x03ffffff)
def M(addr):return ELF_OFF+addr-ELF_BASE

def make_checked():
    a=CHECKED;old=UNCHECKED+5*4;common=a+9*4
    p=[
       I(0x0b,4,8,0xa1),             # 0  sltiu t0,a0,0xA1
       B(0x05,8,0,a+4,common),       # 1  bne t0,zero,common
       I(0x0f,0,3,0x08a7),           # 2  lui v1,0x08A7 (delay for both)
       I(0x09,4,2,-0xa1),            # 3  secondary: addiu v0,a0,-0xA1
       R(0,2,2,1,0),                 # 4  sll v0,v0,1
       I(0x0d,3,3,0xf294),          # 5  ori v1,v1,0xf294 -> 0x08A7F294
       R(3,2,3,0,0x21),             # 6  addu v1,v1,v0
       R(31,0,0,0,0x08),             # 7  jr ra
       I(0x21,3,2,0),                # 8  lh v0,0(v1) delay slot
       I(0x09,3,3,0x43e0),          # 9  common: addiu v1,v1,0x43e0 -> table
       R(3,4,3,0,0x21),             # 10 addu v1,v1,a0
       I(0x24,3,2,0),                # 11 lbu v0,0(v1)
       I(0x0b,2,8,0x80),            # 12 sltiu t0,v0,0x80
       B(0x05,8,0,a+13*4,old),      # 13 bne old
       I(0x0b,2,8,0xaf),            # 14 sltiu t0,v0,0xaf (branch delay)
       B(0x04,8,0,a+15*4,old),      # 15 beq old when FF
       I(0x09,2,2,-0x80),            # 16 addiu v0,v0,-0x80
       R(0,2,3,2,0),                 # 17 sll v1,v0,2
       R(2,3,2,0,0x21),             # 18 addu v0,v0,v1 (*5)
       R(2,5,2,0,0x21),             # 19 addu v0,v0,a1
       I(0x09,2,2,FIRST),           # 20 addiu v0,v0,0x3711
       R(31,0,0,0,0x08),             # 21 jr ra
       0x7C021620,                  # 22 seh v0,v0
       0,                           # 23 nop
    ]
    assert len(p)==24
    return p

def make_unchecked():
    a=UNCHECKED; old=a+5*4
    p=[
      I(0x0b,4,8,0xa1),          # 0 sltiu t0,a0,0xA1
      B(0x04,8,0,a+4,old),      # 1 beq old (id out of range)
      I(0x0f,0,3,0x08a7),       # 2 lui v1,0x08A7 (delay slot)
      J(CHECKED+9*4),           # 3 j common, preserving RA
      0,                        # 4 nop
      I(0x0f,0,3,0x08a7),       # 5 old: lui v1,0x08A7
      I(0x09,3,3,-0x68e0),      # 6 addiu v1,v1,-0x68E0 (group table)
      R(3,4,3,0,0x21),          # 7 addu v1,v1,a0
      I(0x24,3,2,0),             # 8 lbu v0,0(v1)
      R(0,2,3,2,0),             # 9 sll v1,v0,2
      R(2,3,2,0,0x21),          # 10 addu v0,v0,v1
      R(2,5,2,0,0x21),          # 11 addu v0,v0,a1
      I(0x09,2,2,0x0593),      # 12 addiu v0,v0,0x0593
      R(31,0,0,0,0x08),         # 13 jr ra
      0x7C021620,               # 14 seh v0,v0
    ]
    assert len(p)==15
    return p

def patch_elf(blob):
    assert len(blob)==3747908 and hashlib.sha256(blob).hexdigest()==EXPECTED_ELF_SHA, 'Base ELF V0.3 diferente da analisada'
    assert blob[:4]==b'\x7fELF'
    assert struct.unpack_from('<I',blob,M(COUNT_ADDR))[0]==0x341E3734
    e=bytearray(blob)
    for addr,words in ((CHECKED,make_checked()),(UNCHECKED,make_unchecked())):
        struct.pack_into('<'+('I'*len(words)),e,M(addr),*words)
    for slot,bid in enumerate(IDS):
        assert e[M(TEXT_MAP)+bid]==255 and e[M(GROUP_MAP)+bid]==255, f'ID ocupado: {bid:02X}'
        e[M(TEXT_MAP)+bid]=SLOT_CODE_BASE+slot
        e[M(GROUP_MAP)+bid]=0  # resource fallback = Goku Base group
    struct.pack_into('<I',e,M(COUNT_ADDR),0x341e0000|NEW_COUNT)
    assert len(e)==len(blob) and e[:ELF_OFF]==blob[:ELF_OFF]
    assert e[M(COUNT_ADDR):M(COUNT_ADDR)+4]==b'\xfc\x37\x1e\x34'
    assert all(e[M(TEXT_MAP)+bid]==0x80+n for n,bid in enumerate(IDS))
    for bid in USED:
        assert e[M(TEXT_MAP)+bid]==blob[M(TEXT_MAP)+bid]
        assert e[M(GROUP_MAP)+bid]==blob[M(GROUP_MAP)+bid]
    return bytes(e)

def patch_header(blob):
    assert len(blob)==DATA and blob[:4]==b'PacK'
    assert struct.unpack_from('<I',blob,4)[0]==OLD_COUNT, 'Header deve ser 0x3734'
    h=bytearray(blob)
    src=[decode(h,i) for i in range(0x593,0x598)]
    assert all(pair and pair[0]>=DATA and pair[1]>0 for pair in src)
    tmpl=h[16+0x593*16:16+0x594*16]
    for slot in range(7,47): # NOTE: 0-based slot 7 -> INDEX 0x3734. Preserve 35 original extras!
        for var in range(5):
            idx=FIRST+5*slot+var
            assert idx>=OLD_COUNT and idx<NEW_COUNT
            pos=16+idx*16
            assert h[pos:pos+16]==bytes(16),f'Regiao de header nao vazia em {idx:04X}'
            off,size=src[var]
            new_record=encode(idx,off,size,tmpl)
            h[pos:pos+16]=new_record
            if decode(h,idx)!=(off,size):
                # O conversor legado de tamanhos possui um nibble nao bijetivo
                # para certos indices; um CHR de Goku C1 e fallback seguro.
                off,size=src[0]
                h[pos:pos+16]=encode(idx,off,size,tmpl)
            assert decode(h,idx)==(off,size), f'Falha ao codificar indice {idx:04X}' 
    struct.pack_into('<I',h,4,NEW_COUNT)
    assert h[16:16+OLD_COUNT*16]==blob[16:16+OLD_COUNT*16]
    assert struct.unpack_from('<I',h,4)[0]==0x37FC
    assert len(h)==len(blob)
    return bytes(h)

def make_manifest():
    rows=[]
    for slot,bid in enumerate(IDS):
        idx=FIRST+slot*5
        rows.append(dict(slot=f'{slot+1:02d}', battle_id=f'{bid:02X}', id_menu_expansao=f'{SLOT_CODE_BASE+slot:02X}',
             nome=('Goku (Inicio)','Goku (Meio)','Goku (Meio) SSJ')[slot] if slot<3 else f'Personagem {slot+1:02d}',
             chr_color1=f'{idx:04X}',chr_color2=f'{idx+1:04X}',chr_color3=f'{idx+2:04X}',chr_color4=f'{idx+3:04X}',sfx=f'{idx+4:04X}',
             recurso=('CHR adicionado, testado' if slot<3 else 'Placeholder com dados do Goku; ID ainda nao testado no PPSSPP' if slot>=7 else 'Recursos ja existentes no header; nao regravados')))
    return rows

def main():
    import argparse
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--elf-v03',required=True,help='EBOOT.V03_RESOLVER_MESMO_TAMANHO.ELF, SHA verificado')
    ap.add_argument('--header-v03',required=True,help='PACKFILE_CABEÇALHO2.BIN, contagem 0x3734')
    ap.add_argument('--outdir',required=True,help='Pasta NOVA de saida')
    a=ap.parse_args()
    out=Path(a.outdir)
    out.mkdir(exist_ok=True,parents=True)
    e=Path(a.elf_v03).read_bytes()
    p=Path(a.header_v03).read_bytes()
    (out/'EBOOT.V04_47IDS_EXPERIMENTAL.ELF').write_bytes(patch_elf(e))
    (out/'PACKFILE_CABECALHO_47_ALIAS_GOKU.BIN').write_bytes(patch_header(p))
    rows=make_manifest()
    with (out/'MAPA_47_PERSONAGENS.csv').open('w',encoding='utf8',newline='') as f:
        w=csv.DictWriter(f,fieldnames=rows[0].keys());w.writeheader();w.writerows(rows)
    (out/'MAPA_47_PERSONAGENS.json').write_text(json.dumps(rows,indent=2,ensure_ascii=False),encoding='utf8')
    print('OK EBOOT, HEADER e MAPA; ids:', ', '.join(f'{i:02X}' for i in IDS))

if __name__=='__main__':main()
