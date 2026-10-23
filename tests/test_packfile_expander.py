#!/usr/bin/env python3
"""Integration check of CURRENT dlc_packfile_builder (no original game data)."""
from pathlib import Path
import json
import struct
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import dlc_packfile_builder as builder
from inspecionar_packfile import DATA, decode, VERIFIED

with tempfile.TemporaryDirectory() as tmp:
    tmp = Path(tmp)
    header = bytearray(DATA)
    header[:4] = b'PacK'
    struct.pack_into('<I', header, 4, 0x3711)
    template = b'\x00' * 8 + b'PacK' + b'\x00' * 4
    # Create original model/SFX descriptors; offsets are relative to data area.
    for index, offset, size in [(builder.ORIG_CHR, DATA, 0x1000),
                                (builder.ORIG_SFX, DATA + 0x1000, 0x800)]:
        p = 16 + index * 16
        header[p:p+16] = builder.encode(index, offset, size, template)
        assert decode(header, index) == (offset, size)
    original = tmp / 'PACKFILE_original.BIN'
    original.write_bytes(header + b'C' * 0x1000 + b'S' * 0x800)
    assets = tmp / 'assets'
    for form in builder.CHAR_NAMES:
        d = assets / form
        d.mkdir(parents=True)
        for color in range(1, 5):
            (d / f'C{color}.PCK1').write_bytes(b'\x00\x00\x01\xf1' + bytes([color]) * (0x10000 - 4))
    output = tmp / 'PACKFILE_DLC.BIN'
    builder.build(original, output, assets)
    result = output.read_bytes()
    assert struct.unpack_from('<I', result, 4)[0] == VERIFIED
    # Original entries and original CHR/SFX bytes are unaffected.
    assert result[16:16 + 0x3711 * 16] == header[16:16 + 0x3711 * 16]
    assert result[DATA:DATA+0x1800] == original.read_bytes()[DATA:]
    for form_id in range(3):
        for color_index in range(4):
            idx = 0x3711 + form_id * 5 + color_index
            off, length = decode(result[:DATA], idx)
            assert length == 0x10000
            assert result[off:off + 4] == b'\x00\x00\x01\xf1'
        # Each reserved SFX slot aliases original SFX.
        assert decode(result[:DATA], 0x3711 + form_id * 5 + 4) == (DATA + 0x1000, 0x800)
    manifest = json.loads(output.with_suffix('.manifest.json').read_text())
    assert len(manifest['new_entries']) == 47 # 35 aliases iniciais + 12 ajustes fisicos
    assert sum(row['mode'] == 'NEW_CHR' for row in manifest['new_entries']) == 12
    assert len(set(row['index'] for row in manifest['new_entries'])) == 35
print('PASS: DLC builder 12 real CHRs / 35 entries, original files preserved')
