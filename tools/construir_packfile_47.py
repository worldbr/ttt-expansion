#!/usr/bin/env python3
"""Construtor conservador das 44 formas adicionais (slots 04..47).
Não cria CHRs inventados: exige 4 CHRs e 1 SFX para CADA slot.
Entrada: PACKFILE completo já expandido para 0x3734, e 220 recursos reais.
Saída: novo PACKFILE com contagem 0x37FC e novas entradas (NÃO modifica origem).
Isso NÃO habilita os 44 IDs de combate; ainda precisam de engenharia reversa.
"""
import argparse,struct,shutil,os,json
from pathlib import Path
from inspecionar_packfile import decode,DATA
from dlc_packfile_builder import encode,pad
FIRST=0x3711
COUNT=0x37FC

def build(source,destination,assets):
    source=Path(source).resolve();destination=Path(destination).resolve();assets=Path(assets).resolve()
    if source==destination or destination.exists():raise ValueError('A saída deve ser NOVA e não pode existir')
    with source.open('rb') as f:head=bytearray(f.read(DATA))
    if len(head)!=DATA or head[:4]!=b'PacK':raise ValueError('Cabeçalho PACKFILE inválido')
    if source.stat().st_size<=DATA:raise ValueError('Arquivo somente de cabeçalho: é necessário PACKFILE COMPLETO')
    original_count=struct.unpack_from('<I',head,4)[0]
    if original_count!=0x3734:raise ValueError(f'Exige PACKFILE 0x3734 testado, encontrado {original_count:#x}')
    entries=[]
    for pos in range(4,48):
        folder=assets/f'SLOT_{pos:02d}'
        for variant in range(5):
            typ=f'C{variant+1}.PCK1' if variant<4 else 'SFX.PCK1'
            f=folder/typ
            if not f.is_file():raise FileNotFoundError(f'Faltando recurso real: {f}')
            size=f.stat().st_size
            if not 256<=size<0x1000000:raise ValueError(f'Tamanho suspeito em {f}: {size}')
            if variant<4 and f.open('rb').read(4)!=bytes.fromhex('000001f1'):
                raise ValueError(f'Assinatura CHR não reconhecida: {f}')
            entries.append((FIRST+(pos-1)*5+variant,f))
    assert len(entries)==220
    template=head[16+0x593*16:16+0x594*16]
    if decode(head,0x593) is None:raise ValueError('Índice original de CHR inválido')
    result=[]
    try:
        with source.open('rb') as inp,destination.open('xb+') as out:
            shutil.copyfileobj(inp,out,4*1024*1024)
            for idx,f in entries:
                out.seek(0,os.SEEK_END);pad(out)
                off=out.tell();size=f.stat().st_size
                with f.open('rb') as stream:shutil.copyfileobj(stream,out,1024*1024)
                head[16+idx*16:16+(idx+1)*16]=encode(idx,off,size,template)
                if decode(head,idx)!=(off,size):raise RuntimeError(f'Falha verificação do índice {idx:#x}')
                result.append({'idx':hex(idx),'source':str(f),'offset':off,'size':size})
            # Slots 0..2 permanecem como já estavam no PACKFILE testado.
            struct.pack_into('<I',head,4,COUNT)
            out.seek(0);out.write(head)
        destination.with_suffix('.manifest.json').write_text(json.dumps({'count':COUNT,'files':result,'note':'Ainda precisa de EBOOT com contagem 0x37FC e 44 novos IDs de combate'},indent=2,ensure_ascii=False),encoding='utf-8')
    except Exception:
        destination.unlink(missing_ok=True)
        raise
    print(f'OK: {COUNT:#x} entradas; {len(entries)} recursos reais adicionados a {destination}')
    print('ATENÇÃO: O EBOOT V0.3 NÃO implementa os IDs de combate dos slots 4 a 47.')
if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--packfile',required=True);ap.add_argument('--output',required=True);ap.add_argument('--assets',required=True)
    args=ap.parse_args();build(args.packfile,args.output,args.assets)
