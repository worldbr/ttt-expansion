#!/usr/bin/env python3
"""Teste 35 novos slots (alias e append) usando dados sinteticos, sem ISO real."""
from pathlib import Path
import sys, subprocess, tempfile
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import expandir_packfile_7_gokus as x
root=Path(__file__).resolve().parents[1]
base=(root/'EXPANSAO_7_GOKUS'/'CABECALHO_BASE_3712_EXPERIMENTAL.bin').read_bytes()
head,entries,template=x.make_header(base)
assert len(entries)==35 and int.from_bytes(head[4:8],'little')==0x3734
assert head[8:x.entry_idx(x.FIRST)]==base[8:x.entry_idx(x.FIRST)]
assert head[(x.EXPANDED+1)*16:]==base[(x.EXPANDED+1)*16:]
for row in entries:
    assert x.decode_record(head,row['index'])==(row['offset'],row['size'])
assert entries[0]['index']==0x3711 and entries[-1]['index']==0x3733
assert x.decode_record(head,0x3711)==x.decode_record(base,x.ORIGINAL_CHR)
assert x.decode_record(head,0x3715)==x.decode_record(base,x.ORIGINAL_SFX)
with tempfile.TemporaryDirectory() as td:
    temp=Path(td)
    fake=bytearray(base)
    fake[x.entry_idx(x.ORIGINAL_CHR):x.entry_idx(x.ORIGINAL_CHR)+16]=x.encode_record(x.ORIGINAL_CHR,x.DATA,4096,template)
    fake[x.entry_idx(x.ORIGINAL_SFX):x.entry_idx(x.ORIGINAL_SFX)+16]=x.encode_record(x.ORIGINAL_SFX,x.DATA+0x1000,2048,template)
    orig=temp/'orig.bin'; orig.write_bytes(fake+b'\xab'*0x8000)
    item=temp/'assets'/'GOKU_EARLY';item.mkdir(parents=True)
    (item/'C1.PCK1').write_bytes(bytes.fromhex('000001f1')+b'x'*(0x12F220-4))
    out=temp/'expanded.bin'
    cmd=[sys.executable,str(root/'tools'/'expandir_packfile_7_gokus.py'),
         '--input',str(orig),'--output',str(out),'--asset-dir',str(temp/'assets')]
    subprocess.run(cmd,check=True,stdout=subprocess.DEVNULL)
    result=out.read_bytes()
    assert result[:x.entry_idx(x.FIRST)]==orig.read_bytes()[:x.entry_idx(x.FIRST)].replace(
        bytes.fromhex('12 37 00 00'),bytes.fromhex('34 37 00 00'),1)
    off,size=x.decode_record(result[:x.DATA],x.FIRST)
    assert size==0x12F220 and result[off:off+4]==bytes.fromhex('000001f1')
    assert x.decode_record(result[:x.DATA],x.FIRST+1)==(x.DATA,4096)
print('PASS: 7 forms, 35 entries, codec, alias, physical append, original entries preserved')
