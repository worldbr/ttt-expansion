#!/usr/bin/env python3
"""Cria 35 registros PACKFILE para sete formas de Goku (4 CHRs + 1 SFX).
Sem dependencias externas. Codec extraido do Editor_indice.exe enviado pelo usuario.
Nunca altera o arquivo de origem. --asset-dir opcional, por padrao faz alias.
"""
import argparse
import csv
import json
import os
import shutil
import struct
from pathlib import Path
from packfile_codec import DataConvert

ORIGINAL=0x3711
EXPANDED=0x3734
FIRST=0x3711
N=35
DATA=0x38000
SECTOR=0x800
ORIGINAL_CHR=0x593
ORIGINAL_SFX=0x597
MAGIC=b'PacK'
FORMS=('GOKU_EARLY','GOKU_MID','GOKU_MID_SSJ','GOKU_GT','GOKU_GT_SSJ','GOKU_GT_SSJ3','GOKU_GT_SSJ4')

class Field:
    def text(self): return '0x38000'
class Context:
    index_Packfile=[0]
    edit_lbl_data_size=Field()
CODEC=DataConvert(Context())

def entry_idx(i): return 16+16*i

def decode_record(head,i):
    block=head[entry_idx(i):entry_idx(i)+16]
    if len(block)!=16 or block[8:12]!=MAGIC:
        raise ValueError(f'Registro {i:#x} sem PacK/16 bytes')
    text=' '.join(f'{b:02x}' for b in block[:4])
    off=int(CODEC.getOffsetConvert(text),16)
    size=int(CODEC.getSizeConvert(f'{i:x}',block[4:8]),16)
    if off<DATA or (off-DATA)%SECTOR:raise ValueError(f'Offset invalido em {i:#x}: {off:#x}')
    return (off,size)

def encode_record(i,off,size,template):
    if off<DATA or (off-DATA)%SECTOR:raise ValueError('Offset fora do setor de dados/alinhamento')
    if size<1 or size>0xFFFFFFFF:raise ValueError('Tamanho invalido')
    sectors=(off-DATA)//SECTOR
    raw=' '.join(f'{b:02x}' for b in struct.pack('<I',sectors))
    addr=bytes.fromhex(CODEC.getOffsetConvert(raw,False))
    sz=bytes.fromhex(CODEC.getSizeConvert(f'{i:x}',struct.pack('<I',size),False))
    record=addr+sz+template[8:16]
    if len(record)!=16 or record[8:12]!=MAGIC:raise ValueError('Erro ao construir entrada')
    return record

def parse_int(s): return int(s,0)

def slots():
    for formno, name in enumerate(FORMS):
        for kind in ('C1','C2','C3','C4','SFX'):
            n=formno*5+('C1','C2','C3','C4','SFX').index(kind)
            i=FIRST+n
            yield dict(index=i,number=i+1,name=name,kind=kind,
                       asset=f'{name}/{kind}.PCK1')

def make_header(base,asset_location=None,require_full=False):
    if len(base)<DATA:raise ValueError('Arquivo nao contem os primeiros 0x38000 bytes')
    count=struct.unpack_from('<I',base,4)[0]
    if base[:4]!=MAGIC or count not in (ORIGINAL,ORIGINAL+1,EXPANDED):
        raise ValueError(f'Cabecalho inesperado: magic={base[:4]!r}, count={count:#x}')
    head=bytearray(base[:DATA]); chr_off,chr_size=decode_record(head,ORIGINAL_CHR)
    sfx_off,sfx_size=decode_record(head,ORIGINAL_SFX)
    template=head[entry_idx(ORIGINAL_CHR):entry_idx(ORIGINAL_CHR)+16]
    info=[]
    for row in slots():
        off,size=(sfx_off,sfx_size) if row['kind']=='SFX' else (chr_off,chr_size)
        row.update(offset=off,size=size,mode='ALIAS',reference='SFX_GOKU' if row['kind']=='SFX' else 'CHR_GOKU')
        if asset_location:
            asset=Path(asset_location)/row['asset']
            if asset.is_file():
                if not asset.stat().st_size:raise ValueError(f'Asset vazio: {asset}')
                row.update(mode='NOVO',reference='ARQUIVO_EXTERNO',asset_abs=str(asset))
            elif require_full:raise FileNotFoundError(f'Asset faltando: {asset}')
        info.append(row)
    struct.pack_into('<I',head,4,EXPANDED)
    # Entries only populated here after append; alias baseline created first.
    for row in info:
        rec=encode_record(row['index'],row['offset'],row['size'],template)
        head[entry_idx(row['index']):entry_idx(row['index'])+16]=rec
        if decode_record(head,row['index'])!=(row['offset'],row['size']):
            raise AssertionError(f'Codec nao passou no round-trip: {row["index"]:#x}')
    if head[(EXPANDED+1)*16:] != bytearray(base[(EXPANDED+1)*16:DATA]):
        raise AssertionError('Alteracao involuntaria do final da area reservada')
    return head,info,template

def manifest(out,items):
    p=Path(out)
    payload=[]
    for r in items:
        d={k:v for k,v in r.items() if k!='asset_abs'}
        d['index_hex']=f'0x{r["index"]:04X}'
        d['offset_hex']=f'0x{r["offset"]:X}'
        d['size_hex']=f'0x{r["size"]:X}'
        payload.append(d)
    p.with_suffix('.json').write_text(json.dumps(dict(total=EXPANDED,hex_count='0x3734',
        notes='GT external IDs still unknown; SFX indexed but not hooked.',
        slots=payload),indent=2,ensure_ascii=False),encoding='utf-8')
    with p.with_suffix('.csv').open('w',encoding='utf-8-sig',newline='') as f:
        fields=('number','index_hex','name','kind','mode','reference','asset','offset_hex','size_hex')
        w=csv.DictWriter(f,fieldnames=fields,extrasaction='ignore');w.writeheader();w.writerows(payload)

def pad_to_sector(f):
    x=f.tell();add=(-x)%SECTOR
    if add:f.write(b'\0'*add)

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--input',required=True,help='PACKFILE.BIN ou cabecalho de 0x38000 bytes')
    ap.add_argument('--output',required=True,help='Arquivo NOVO de saida (nao sobrescreve entrada)')
    ap.add_argument('--asset-dir',help='Pasta com GOKU_EARLY/C1.PCK1 etc. Requer PACKFILE.BIN completo')
    ap.add_argument('--require-all-assets',action='store_true',help='Exige os 35 arquivos reais')
    a=ap.parse_args()
    if a.require_all_assets and not a.asset_dir:ap.error('--require-all-assets requer --asset-dir')
    src=Path(a.input).resolve();out=Path(a.output).resolve()
    if src==out:ap.error('Entrada e saida nao podem ser o mesmo arquivo!')
    if not src.is_file():ap.error('Entrada nao encontrada')
    if out.exists():ap.error('Saida existente; escolha outro nome, preservando backups')
    with src.open('rb') as f:base=f.read(DATA)
    hdr,items,template=make_header(base,a.asset_dir,a.require_all_assets)
    if a.asset_dir and len(base)==src.stat().st_size:
        ap.error('Assets requerem PACKFILE.BIN completo, nao apenas o cabecalho')
    # Validar que os aliases apontam para dados presentes no PACKFILE real.
    if src.stat().st_size>DATA:
        for i in (ORIGINAL_CHR,ORIGINAL_SFX):
            off,size=decode_record(base,i)
            if off+size>src.stat().st_size:
                ap.error(f'Arquivo de entrada truncado: recurso fonte {i:#x} fora dos limites')
    try:
        if src.stat().st_size==DATA:
            if any(x['mode']=='NOVO' for x in items):ap.error('Nao pode anexar assets usando apenas cabecalho')
            with out.open('xb') as f:f.write(hdr)
        else:
            with src.open('rb') as inf,out.open('xb+') as fout:
                shutil.copyfileobj(inf,fout,length=4*1024*1024)
                for row in items:
                    if row['mode']!='NOVO':continue
                    fout.seek(0,os.SEEK_END)
                    pad_to_sector(fout)
                    offset=fout.tell()
                    if offset<DATA or (offset-DATA)%SECTOR:raise AssertionError('Alinhamento')
                    asset=Path(row['asset_abs'])
                    row['offset']=offset;row['size']=asset.stat().st_size
                    with asset.open('rb') as res:shutil.copyfileobj(res,fout,length=4*1024*1024)
                    new_record=encode_record(row['index'],row['offset'],row['size'],template)
                    hdr[entry_idx(row['index']):entry_idx(row['index'])+16]=new_record
                    if decode_record(hdr,row['index'])!=(row['offset'],row['size']):
                        raise AssertionError('Roundtrip falhou com asset real')
                fout.seek(0);fout.write(hdr)
        manifest(out,items)
    except Exception:
        if out.exists():out.unlink()
        raise
    print(f'OK: {out.name} | {EXPANDED} entradas (0x{EXPANDED:X}) | '
          f'35 novos slots: {sum(i["mode"]=="NOVO" for i in items)} assets fisicos, '
          f'{sum(i["mode"]=="ALIAS" for i in items)} aliases')
    print(f'Indice restante livre: {(DATA-(EXPANDED+1)*16)//16} registros. EBOOT: patch 0x3711/0x3712 para 0x3734 ANTES de iniciar.')
    print('AVISO: para ISO, reconstrua ISO9660 atualizando tamanho de PACKFILE.BIN; NAO substitua bytes cegamente.')

if __name__=='__main__':main()
