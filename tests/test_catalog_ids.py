import sys,re
from pathlib import Path
p=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(p/'tools'))
from gerar_eboot_packfile_v04 import IDS
names=(p/'TTT_EXPANSION/characters.txt').read_text(encoding='utf8')
entries=[line.split('=',1) for line in names.splitlines() if '=' in line and not line.startswith('#')]
assert len(entries)==47
assert [int(k) for k,v in entries]==list(range(1,48))
s=(p/'src/character_ids.h').read_text()
assert tuple(int(t,16) for t in re.findall(r'0x([0-9A-F]{2})u',s))[:47]==IDS
assert len(IDS)==len(set(IDS))==47
print('OK: 47 distinct IDs and 47 editable names')
