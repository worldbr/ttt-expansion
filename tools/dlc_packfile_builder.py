#!/usr/bin/env python3
"""Cria um novo PACKFILE a partir da contagem ORIGINAL 0x3711/0x3712.
Nunca sobrescreve o original, e recusa PACKFILE ja expandido (0x3734).
Necessita os 12 CHRs em assets/ para preservar diferencas entre Colors.
"""
import argparse,csv,json,os,shutil,struct
from pathlib import Path
from inspecionar_packfile import C, DATA, ORIGINAL, VERIFIED, MAX, decode
FIRST=0x3711; SECTOR=0x800
CHAR_NAMES=('GOKU_INICIO','GOKU_MEIO','GOKU_MEIO_SSJ')
ORIG_CHR=0x593; ORIG_SFX=0x597

def encode(i,off,size,template):
    if off<DATA or (off-DATA)%SECTOR or not 0<size<=0xffffffff:raise ValueError('Offset ou tamanho invalido')
    sector=(off-DATA)//SECTOR
    v=' '.join('%02x'%b for b in struct.pack('<I',sector))
    return bytes.fromhex(C.getOffsetConvert(v,False))+bytes.fromhex(C.getSizeConvert('%x'%i,struct.pack('<I',size),False))+template[8:16]
def pad(f):
    n=(-(f.tell()-DATA))%SECTOR
    if n:f.write(b'\0'*n)
def build(source,target,asset_dir):
    source=Path(source).resolve();target=Path(target).resolve();asset_dir=Path(asset_dir).resolve()
    if source==target or target.exists():raise ValueError('Destino precisa ser novo/diferente do arquivo original')
    with source.open('rb') as f:base=f.read(DATA)
    if len(base)!=DATA or base[:4]!=b'PacK':raise ValueError('PACKFILE original com header valido necessario')
    count=struct.unpack_from('<I',base,4)[0]
    if count not in (0x3711,0x3712):raise ValueError('RECUSADO: nao reempacote PACKFILE ja expandido. Preserve sua ISO existente!')
    if source.stat().st_size<=DATA:raise ValueError('Necessario PACKFILE completo, nao somente cabecalho')
    chrval=decode(base,ORIG_CHR);sfxval=decode(base,ORIG_SFX)
    if not chrval or not sfxval:raise ValueError('CHR/SFX fonte inexistentes')
    for off,size in [chrval,sfxval]:
        if off+size>source.stat().st_size:raise ValueError('Arquivo fonte truncado')
    assets=[]
    for form_id,name in enumerate(CHAR_NAMES):
        for color in range(1,5):
            asset=asset_dir/name/('C%d.PCK1'%color)
            if not asset.is_file():raise FileNotFoundError('Falta CHR requerido: '+str(asset))
            size=asset.stat().st_size
            if not 0x10000<=size<=0x300000:raise ValueError('Tamanho CHR fora do limite conservador 64KiB..3MiB: '+str(asset))
            with asset.open('rb') as f:
                if f.read(4)!=bytes.fromhex('000001f1'):raise ValueError('Assinatura CHR nao reconhecida: '+str(asset))
            assets.append((FIRST+form_id*5+color-1,asset))
    header=bytearray(base)
    template=base[16+ORIG_CHR*16:16+(ORIG_CHR+1)*16]
    rows=[]
    struct.pack_into('<I',header,4,VERIFIED)
    def set_rec(index,off,size,mode):
        hdr=encode(index,off,size,template)
        pos=16+index*16;header[pos:pos+16]=hdr
        assert decode(header,index)==(off,size)
        rows.append({'index':index,'entry_number':index+1,'mode':mode,'offset':off,'size':size})
    # Create all 35 records. Unused slots are safe aliases of known resource
    # indexes until separately validated. Do not rewrite ORIGINAL records.
    for i in range(FIRST,VERIFIED):
        is_sfx=(i-FIRST)%5==4
        off,size=sfxval if is_sfx else chrval
        set_rec(i,off,size,'ALIAS_RESERVED')
    try:
        with source.open('rb') as inp,target.open('xb+') as out:
            shutil.copyfileobj(inp,out,4*1024*1024)
            for i,asset in assets:
                out.seek(0,os.SEEK_END);pad(out)
                off=out.tell();size=asset.stat().st_size
                with asset.open('rb') as f:shutil.copyfileobj(f,out,1024*1024)
                set_rec(i,off,size,'NEW_CHR')
            out.seek(0);out.write(header)
        result={'count':VERIFIED,'count_hex':hex(VERIFIED),'original_sha256':'not modified','new_entries':rows,
                'warning':'O EBOOT descriptografado ainda requer patch de contador 0x3734; depois reconstrua ISO9660.'}
        target.with_suffix('.manifest.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8')
    except Exception:
        target.unlink(missing_ok=True)
        raise
    print('OK: 12 CHRs fisicos + 35 entradas (contagem 0x3734) ->',target)
    print('NAO coloque no ISO por HxD: reconstrua imagem ISO9660 atualizando tamanho do arquivo.')

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--packfile',required=True);p.add_argument('--output',required=True)
    p.add_argument('--assets',required=True,help='Pasta com GOKU_INICIO/C1.PCK1 ... GOKU_MEIO_SSJ/C4.PCK1')
    a=p.parse_args();build(a.packfile,a.output,a.assets)
if __name__=='__main__':main()
