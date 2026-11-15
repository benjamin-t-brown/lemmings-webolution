"""Compare source-built clone births and embed their literal PNGs in an SVG.

No original execution, asset mutation, packages, capture, or generated pixels.
"""
import base64
import csv
import hashlib
import json
from pathlib import Path
import struct
import sys
import zlib


def hatch_pixels(data):
    chunks, at = {}, 8
    while at < len(data):
        size = struct.unpack_from('>I', data, at)[0]
        kind = data[at+4:at+8]
        chunks[kind] = chunks.get(kind, b'') + data[at+8:at+8+size]
        at += size + 12
    w, h, depth, kind, compression, filtering, interlace = struct.unpack('>IIBBBBB', chunks[b'IHDR'])
    assert (w, h, depth, compression, filtering, interlace) == (640, 480, 8, 0, 0, 0)
    assert kind in (2, 6)
    channels = 3 if kind == 2 else 4
    raw = zlib.decompress(chunks[b'IDAT'])
    stride = 1 + w * channels
    result = []
    # The clone PNG writer uses unfiltered scanlines. Inspect only the hatch
    # crop; wall-clock scenery elsewhere is deliberately not compared.
    for y in range(24, 132):
        assert raw[y*stride] == 0
        for x in range(274, 364):
            offset = y*stride + 1 + x*channels
            result.append(raw[offset:offset+3])
    return result


def main(root):
    out = root / 'build/hatch-release-direction-001'
    levels = ('MM_W_012', 'MM_CLIMB_002', 'MM_BASH_002')
    wanted = ((1, -1, 1, -1, 1, -1), (1,) * 6, (-1,) * 6)
    parts = ['<svg xmlns="http://www.w3.org/2000/svg" width="1100" height="1460">',
             '<rect width="100%" height="100%" fill="#171b22"/>',
             '<g fill="white" font-family="sans-serif" font-size="15">',
             '<text x="12" y="22">Actual clone births: retained before / corrected after. R = right, L = left.</text>']
    records = []
    for route, (level, expected) in enumerate(zip(levels, wanted)):
        rows = {}
        for side in ('before', 'after'):
            with (out / side / level / 'births.csv').open(newline='') as file:
                rows[side] = list(csv.DictReader(file))
            assert len(rows[side]) == 6
        assert tuple(int(row['facing']) for row in rows['before']) == (1,) * 6
        assert tuple(int(row['facing']) for row in rows['after']) == expected
        for n, (old, new) in enumerate(zip(rows['before'], rows['after']), 1):
            assert {k: v for k, v in old.items() if k != 'facing'} == {
                k: v for k, v in new.items() if k != 'facing'}, (level, n)
        records.append({'level': level, 'before': rows['before'], 'after': rows['after'], 'images': []})
        for n in range(1, 7):
            old = hatch_pixels((out / 'before' / level / f'birth-{n}.png').read_bytes())
            new = hatch_pixels((out / 'after' / level / f'birth-{n}.png').read_bytes())
            changed = sum(a != b for a, b in zip(old, new))
            assert bool(changed) == (expected[n-1] == -1), (level, n, changed)
            records[-1].setdefault('crop_changed_pixels', []).append(changed)
        for side_index, side in enumerate(('before', 'after')):
            top = 48 + route * 470 + side_index * 224
            parts.append(f'<text x="12" y="{top}">{level} / {side}</text>')
            for n, row in enumerate(rows[side], 1):
                path = out / side / level / f'birth-{n}.png'
                data = path.read_bytes()
                assert data[:8] == b'\x89PNG\r\n\x1a\n'
                records[-1]['images'].append({'side': side, 'birth': n, 'sha256': hashlib.sha256(data).hexdigest()})
                x = 12 + (n-1) * 180
                face = 'R' if row['facing'] == '1' else 'L'
                parts.append(f'<text x="{x}" y="{top+20}">#{n} {face}, {row["level_clock_ms"]} ms</text>')
                uri = 'data:image/png;base64,' + base64.b64encode(data).decode('ascii')
                parts.append(f'<svg x="{x}" y="{top+26}" width="168" height="180" viewBox="274 24 90 108">'
                             f'<image href="{uri}" width="640" height="480"/></svg>')
    parts.append('</g></svg>')
    (out / 'birth-comparison.svg').write_text('\n'.join(parts) + '\n', encoding='utf-8')
    (out / 'birth-comparison.json').write_text(json.dumps(records, indent=2) + '\n', encoding='utf-8')
    print('18 paired births: exact clocks, coordinates, state and species preserved; expected facings confirmed.')


if __name__ == '__main__':
    main(Path(sys.argv[1]).resolve())
