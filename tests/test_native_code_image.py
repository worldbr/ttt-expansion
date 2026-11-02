#!/usr/bin/env python3
"""Static integrity check for the two V0.4 ELF resolver images.

Expected SHA256s were computed from EBOOT.V04_47IDS_EXPERIMENTAL.ELF
at 0x089C6CA0 (96 bytes) and 0x089C6D60 (60 bytes).
"""
from pathlib import Path
import re, hashlib, struct
r=Path(__file__).resolve().parents[1]
s=(r/'src/native_code_repair.c').read_text()
for name,count,expected in (
    ('checked',24,'40a84d5b14bf320b8d338e548b36ef77c7c531a3b262bed74758ac15a45ef4e9'),
    ('unchecked',15,'bfef8fdadcbe65b7fda136fbeae4bad21aaccb4f92d742754f07011adf63ab09'),
):
    m=re.search(rf'{name}_expected\[.*?\]=\{{(.*?)\}};',s,re.S)
    assert m, name
    words=[int(x,16) for x in re.findall(r'0x([0-9A-Fa-f]{8})u',m.group(1))]
    assert len(words)==count, (name,len(words))
    observed=hashlib.sha256(struct.pack('<'+str(count)+'I',*words)).hexdigest()
    assert observed==expected, (name,observed)
main=(r/'src/main.c').read_text()
assert 'v04_repair_checked_opcode()' in main
assert 'sceKernelIcacheInvalidateRange((void*)(uintptr_t)0x089C6CA0u,4u)' in main
assert 'ACTOR_RESULT J=' in main
assert 'EXPECTED_RESOURCE slot=' in main
print('PASS V0.4.4: EBOOT V0.4 code images match byte-exact hashes, trace hooks wired')
