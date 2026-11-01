"""Structural coverage of all 47 groups + reject missing actual CHR/SFX."""
from pathlib import Path
import json,struct,tempfile,sys
p=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(p/'tools'))
from dlc_packfile_builder import encode
from inspecionar_packfile import decode,DATA
from construir_packfile_47 import build
plan=json.loads((p/'PLANO_47_SLOTS.json').read_text())
assert len(plan['groups'])==47 and plan['count']==0x37fc
assert [g['packfile_chr_indexes_hex'][0] for g in plan['groups']] == [f'0x{0x3711+i*5:04X}' for i in range(47)]
assert plan['groups'][-1]['packfile_sfx_index_hex']=='0x37FB'
with tempfile.TemporaryDirectory() as tmp:
 tmp=Path(tmp)
 h=bytearray(DATA);h[:4]=b'PacK';struct.pack_into('<I',h,4,0x3734)
 # Build a valid source file with already-expanded aliases
 template=b'\0'*8+b'PacK'+b'\0'*4
 src_index=0x593
 h[16+src_index*16:16+(src_index+1)*16]=encode(src_index,DATA,2048,template)
 base=tmp/'test_source.bin';base.write_bytes(h+b'A'*2048)
 try:
  build(base,tmp/'refuse.bin',tmp/'no_assets')
  raise AssertionError('Should refuse missing resources')
 except FileNotFoundError:pass
 assets=tmp/'assets'
 for i in range(4,48):
  d=assets/f'SLOT_{i:02d}';d.mkdir(parents=True)
  for j in range(1,5):(d/f'C{j}.PCK1').write_bytes(b'\0\0\x01\xf1'+bytes([j])*252)
  (d/'SFX.PCK1').write_bytes(b'\0\0\0\x02'+b'Z'*252)
 output=tmp/'pack47.bin';build(base,output,assets)
 with output.open('rb') as f:out_head=f.read(DATA)
 assert struct.unpack_from('<I',out_head,4)[0]==0x37FC
 assert out_head[16:16+0x3711*16]==h[16:16+0x3711*16]
 for pos in (4,22,47):
  for j in range(5):
   idx=0x3711+(pos-1)*5+j
   x=decode(out_head,idx)
   assert x and x[1]==256 and x[0]+x[1]<=output.stat().st_size
 print('OK PACKFILE47: 47 groups, 220 files, index ends at 0x37FB; missing files rejected')
