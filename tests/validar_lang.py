#!/usr/bin/env python3
"""Check keys and value boundaries of PSP UI strings (only Python stdlib)."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
header=(root/'src/ui_text.h').read_text(encoding='utf-8')
keys=set(re.findall(r'TTT_TEXT_([A-Z][A-Z0-9_]*)\s*,',header))
assert len(keys)>30,len(keys)
for code in ('pt','en','es'):
    path=root/'TTT_EXPANSION'/'lang'/f'{code}.txt'
    found=set()
    for line_no,line in enumerate(path.read_text(encoding='utf-8').splitlines(),1):
        line=line.strip()
        if not line or line.startswith(('#',';')):continue
        assert '=' in line,(code,line_no,'missing =')
        key,value=[x.strip() for x in line.split('=',1)]
        assert key in keys,(code,line_no,'unknown key',key)
        assert key not in found,(code,line_no,'duplicate',key)
        assert value,(code,line_no,'empty value',key)
        assert len(value.encode('utf-8'))<=90,(code,line_no,'too long',key)
        found.add(key)
    assert found==keys,(code,'missing',sorted(keys-found),'extra',sorted(found-keys))
    print(f'{code}.txt OK - {len(found)} editable UI texts')
