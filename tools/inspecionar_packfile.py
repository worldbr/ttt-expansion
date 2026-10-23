#!/usr/bin/env python3
"""Analise somente leitura do PACKFILE original/expandido de ULUS10537.
Nao reutiliza automaticamente entradas supostamente vazias: alias pode ser usado.
"""
import argparse, collections, hashlib, json, struct
from pathlib import Path
from packfile_codec import DataConvert
DATA=0x38000
MAX=(DATA-16)//16
ORIGINAL=0x3711
VERIFIED=0x3734
FIRST=0x3711

class Field:
    def text(self):return '0x38000'
class Context:
    index_Packfile=[0]
    edit_lbl_data_size=Field()
C=DataConvert(Context())
def decode(head,i):
    pos=16+i*16;raw=head[pos:pos+16]
    if len(raw)!=16 or raw[8:12]!=b'PacK':return None
    try:
        off=int(C.getOffsetConvert(' '.join('%02x'%v for v in raw[:4])),16)
        size=int(C.getSizeConvert('%x'%i,raw[4:8]),16)
    except (ValueError,IndexError,KeyError):return None
    return (off,size)
def inspect(path):
    p=Path(path)
    with p.open('rb') as f:head=f.read(DATA)
    if len(head)!=DATA or head[:4]!=b'PacK':raise ValueError('Arquivo nao parece ser PACKFILE.BIN completo/cabecalho valido')
    count=struct.unpack_from('<I',head,4)[0]
    if count<ORIGINAL or count>MAX:raise ValueError('Contagem fora dos limites conhecidos do header: %x'%count)
    size=p.stat().st_size
    duplicates=collections.defaultdict(list);invalid=[];zero=[];safe=[]
    for i in range(count):
        val=decode(head,i)
        if val is None:invalid.append(i);continue
        off,length=val
        if not length:zero.append(i)
        if off<DATA or (off-DATA)%0x800 or (size>DATA and off+length>size):
            invalid.append(i)
        if length:duplicates[(off,length)].append(i)
    for char in range(3):
        for color in range(4):
            i=FIRST+5*char+color
            val=decode(head,i) if i<count else None
            safe.append({'external_id':char,'color':color+1,'index':i,
                'index_hex':hex(i),'entry_number':i+1,
                'offset':(val[0] if val else None),'size':(val[1] if val else None),
                'physical_in_file':bool(val and val[0]>=DATA and val[1]>0 and size>DATA and val[0]+val[1]<=size)})
    # Never assume alias means unused; even zero length needs in-game validation.
    return {'path':str(p),'packfile_bytes':size,'header_bytes':DATA,'count':count,
        'count_hex':hex(count),'max_header_count':MAX,'max_theoretical_added':MAX-ORIGINAL,
        'count_extra':count-ORIGINAL,'remaining_header_records':MAX-count,
        'tested_count':VERIFIED,'unused_zero_length_candidates':zero[:100],
        'unused_zero_length_count':len(zero),'invalid_sample':invalid[:24],
        'invalid_count':len(invalid),'aliases_sample':[{'offset':hex(k[0]),'size':hex(k[1]),'indices':v[:8],'count':len(v)} for k,v in duplicates.items() if len(v)>1][:20],
        'characters':safe,'notice':'Espaco de header e teorico; validado no jogo apenas ate 0x3734. Nao reutilize indices originais automaticamente.'}
def main():
    a=argparse.ArgumentParser(description=__doc__)
    a.add_argument('--packfile',required=True)
    a.add_argument('--json',dest='json_out')
    args=a.parse_args()
    data=inspect(args.packfile)
    print('PACKFILE: %s | %s registros (%s)'%(data['path'],data['count'],data['count_hex']))
    print('Original 0x3711 | confirmado no PPSSPP: 0x3734 | teorico max header 0x37FF (238 extras)')
    print('Faltam %d slots no header; candidatos zero-length: %d (nao reutilizar sem pesquisa)'%(data['remaining_header_records'],data['unused_zero_length_count']))
    for c in data['characters']:
        print('ID %02d Color%d: index=%s entry=%d offset=%s size=%s valid_file=%s'%(c['external_id'],c['color'],c['index_hex'],c['entry_number'],hex(c['offset']) if c['offset'] is not None else '-',hex(c['size']) if c['size'] is not None else '-',c['physical_in_file']))
    if args.json_out:Path(args.json_out).write_text(json.dumps(data,indent=2,ensure_ascii=False),encoding='utf-8')
if __name__=='__main__':main()
