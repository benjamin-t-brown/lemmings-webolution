"""Compare source-clone hover frames; reads retained images, never launches a game."""
import csv
import hashlib
import json
from pathlib import Path
import struct
import sys
import zlib


def pixels(path):
    data = path.read_bytes()
    assert data[:8] == b'\x89PNG\r\n\x1a\n'
    chunks, at = {}, 8
    while at < len(data):
        size = struct.unpack_from('>I', data, at)[0]
        tag = data[at + 4:at + 8]
        chunks[tag] = chunks.get(tag, b'') + data[at + 8:at + 8 + size]
        at += size + 12
    w, h, depth, kind, compression, filtering, interlace = struct.unpack('>IIBBBBB', chunks[b'IHDR'])
    assert depth == 8 and kind in (2, 6) and compression == filtering == interlace == 0
    channels = 3 if kind == 2 else 4
    raw, rows = zlib.decompress(chunks[b'IDAT']), []
    for y in range(h):
        row = raw[y * (1 + w * channels):(y + 1) * (1 + w * channels)]
        assert row[0] == 0  # Production PNG writer uses unfiltered rows.
        rows.extend(tuple(row[1 + x * channels:1 + x * channels + 3]) for x in range(w))
    return w, h, rows


def compare(binary_dir, output):
    before = max((binary_dir / 'map-hover-text-before-test').glob('run-*'), key=lambda p: p.stat().st_mtime_ns)
    after = max((binary_dir / 'map-hover-test').glob('run-*'), key=lambda p: p.stat().st_mtime_ns)
    read = lambda p: list(csv.DictReader(p.open(newline='')))
    old, new = read(before / 'frames.csv'), read(after / 'frames.csv')
    assert len(old) == len(new)
    off, hover = 0, 0
    for a, b in zip(old, new):
        assert {k: v for k, v in a.items() if k != 'composed_hash'} == {
            k: v for k, v in b.items() if k != 'composed_hash'}
        if a['node'] == '-1':
            assert a['composed_hash'] == b['composed_hash'], a
            off += 1
        else:
            assert a['composed_hash'] != b['composed_hash'], a
            hover += 1
    assert off == 114 and hover == 109
    images = []
    for a in sorted(before.glob('*.png')):
        b = after / a.name
        assert b.exists(), b
        w, h, ia = pixels(a)
        ww, hh, ib = pixels(b)
        assert (w, h) == (ww, hh)
        changed = [i for i, (x, y) in enumerate(zip(ia, ib)) if x != y]
        bbox = None
        if changed:
            bbox = [min(i % w for i in changed), min(i // w for i in changed),
                    max(i % w for i in changed) + 1, max(i // w for i in changed) + 1]
            assert bbox[0] >= 0 and bbox[1] >= 0 and bbox[2] <= 640 and bbox[3] <= 91
        else:
            assert a.read_bytes() == b.read_bytes()
        images.append({'name': a.name, 'difference_bbox': bbox,
                       'before_sha256': hashlib.sha256(a.read_bytes()).hexdigest(),
                       'after_sha256': hashlib.sha256(b.read_bytes()).hexdigest()})
    for directory in (before, after):
        summary = dict(line.split('=', 1) for line in (directory / 'summary.txt').read_text().splitlines())
        assert summary['failures'] == '0' and summary['selectable_nodes'] == '102'
        assert summary['installed_save_unchanged'] == summary['normal_save_unchanged'] == '1'
    result = {'before': str(before), 'after': str(after), 'unchanged_no_hover_frames': off,
              'changed_hover_frames': hover, 'images': images}
    output.mkdir(parents=True, exist_ok=True)
    (output / 'image-comparison.json').write_text(json.dumps(result, indent=2) + '\n')
    print(f'{off} unchanged no-hover frames; {hover} changed hover frames; {len(images)} pixel-compared image pairs')


if __name__ == '__main__':
    compare(Path(sys.argv[1]), Path(sys.argv[2]))
