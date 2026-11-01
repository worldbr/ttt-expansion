#!/usr/bin/env python3
"""Substitui apenas o header de 224KiB do PACKFILE completo numa copia NOVA.
Nao aumenta o arquivo, nem cria CHRs/SFX; os registros novos apontam a Goku original.
Exige o header original exato que o usuario enviou no contexto da V0.4.
"""
import argparse,hashlib,shutil,struct
from pathlib import Path
from inspecionar_packfile import decode,DATA
OLD=0x3734
NEW=0x37FC
BASE_HASH='718b18ed45237f5a97a7ab310c9b2e9fec92dc757c483c19a184ccdb0e90290f'

def run(source,header,output):
    source=Path(source).resolve();header=Path(header).resolve();output=Path(output).resolve()
    if output==source or output.exists():raise ValueError('Saida deve ter outro nome e NAO existir. Original sera preservado.')
    if not source.is_file():raise FileNotFoundError(source)
    original_size=source.stat().st_size
    if original_size<=DATA:raise ValueError('Voce forneceu apenas o cabecalho: necessario PACKFILE completo para esta etapa.')
    with source.open('rb') as f:old=f.read(DATA)
    new=header.read_bytes()
    if len(new)!=DATA:raise ValueError('Header de saida deve ter 0x38000 bytes')
    if hashlib.sha256(old).hexdigest()!=BASE_HASH:raise ValueError('O header do PACKFILE completo difere do PACKFILE_CABEÇALHO2.BIN fornecido; recusado para proteger a ISO.')
    if struct.unpack_from('<I',old,4)[0]!=OLD or struct.unpack_from('<I',new,4)[0]!=NEW:raise ValueError('Contagens inesperadas')
    if old[16:16+OLD*16]!=new[16:16+OLD*16]:raise ValueError('Nao e permitido modificar os registros existentes')
    for i in range(NEW):
        pair=decode(new,i)
        if pair is None or pair[0]+pair[1]>original_size:
            raise ValueError(f'Registro {i:04X} nao corresponde ao arquivo completo (offset/tamanho fora do arquivo)')
    try:
        with source.open('rb') as inp,output.open('xb+') as out:
            shutil.copyfileobj(inp,out,4*1024*1024)
            out.seek(0);out.write(new)
        if output.stat().st_size!=original_size:raise RuntimeError('Tamanho do PACKFILE mudou, abortando')
    except Exception:
        output.unlink(missing_ok=True)
        raise
    print('OK: PACKFILE completo copiado com header 0x37FC, sem alterar os dados fisicos.')
    print('Este arquivo so deve ser usado com EBOOT V0.4 e ISO reconstruida; teste numa copia.')
if __name__=='__main__':
    a=argparse.ArgumentParser(description=__doc__)
    a.add_argument('--packfile',required=True,help='PACKFILE completo da ISO expandida V0.3')
    a.add_argument('--header',required=True,help='PACKFILE_CABECALHO_47_ALIAS_GOKU.BIN')
    a.add_argument('--output',required=True,help='novo PACKFILE de saida; nao pode existir')
    v=a.parse_args()
    run(v.packfile,v.header,v.output)
