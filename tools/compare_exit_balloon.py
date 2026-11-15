"""Retain R4 image comparisons; only reads clone PNGs and existing reference sheets.

No target launch, capture, external packages or image resynthesis. SVG embeds
the literal inputs, cropping only the reference contact-sheet tiles. This is
a visual comparison at similar cameras, not a full-frame fidelity score.
"""
import base64
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
    raw = zlib.decompress(chunks[b'IDAT'])
    result = []
    for y in range(h):
        row = raw[y * (1+w*channels):(y+1)*(1+w*channels)]
        assert row[0] == 0  # The source clone PNG writer emits unfiltered rows.
        result.extend(tuple(row[1+x*channels:1+x*channels+3]) for x in range(w))
    return w, h, result


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def uri(path, mime='image/png'):
    return f'data:{mime};base64,' + base64.b64encode(path.read_bytes()).decode('ascii')


def main(root):
    out = root / 'build/exit-balloon-path-001'
    before, after = out / 'before', out / 'after'
    assert (before/'simulation.txt').read_bytes() == (after/'simulation.txt').read_bytes()
    assert (before/'SavedGame.dat').read_bytes() == (after/'SavedGame.dat').read_bytes()
    assert (before/'pending-0.png').read_bytes() == (after/'pending-0.png').read_bytes()
    summaries = {}
    for side in ('before', 'after'):
        s = dict(line.split('=', 1) for line in (out/side/'summary.txt').read_text().splitlines())
        assert s['success'] == '1' and s['failures'] == '0'
        assert s['normal_save_unchanged'] == s['installed_save_unchanged'] == '1'
        summaries[side] = s
    references = [root/'docs/playtest/reference/original-2026-11-14'/name
                  for name in ('balloon-a.jpg', 's2-balloon.jpg')]
    records = []
    for ms in range(0, 6001, 500):
        a, b = before/f'pending-{ms}.png', after/f'pending-{ms}.png'
        w, h, old = pixels(a)
        ww, hh, new = pixels(b)
        assert (w, h) == (ww, hh) == (640, 480)
        changed = [i for i, (x, y) in enumerate(zip(old, new)) if x != y]
        bbox = None
        if changed:
            bbox = [min(i % w for i in changed), min(i // w for i in changed),
                    max(i % w for i in changed)+1, max(i // w for i in changed)+1]
            assert bbox[0] >= 480 and bbox[1] >= 50 and bbox[3] <= 350, (ms, bbox)
        assert bool(changed) == (ms != 0)
        records.append({'seconds': ms/1000, 'changed_pixels': len(changed),
                        'difference_bbox': bbox, 'before_sha256': digest(a), 'after_sha256': digest(b)})

    def montage(name, times):
        parts = [f'<svg xmlns="http://www.w3.org/2000/svg" width="1280" height="{50+len(times)*264}" viewBox="0 0 1280 {50+len(times)*264}">',
                 '<rect width="100%" height="100%" fill="#171b22"/>',
                 '<g fill="white" font-family="sans-serif" font-size="14">',
                 '<text x="12" y="20">Retained original 1 (13.5s + t)</text>',
                 '<text x="332" y="20">Clone before: vertical</text>',
                 '<text x="652" y="20">Clone after: mapped departure</text>',
                 '<text x="972" y="20">Retained original 2 (83.5s + t)</text>',
                 '<text x="12" y="42">MM_CLIMB_002 / Just Climb. Similar fixed cameras; clone t=0 is Pending. Original alignment is approximate (2 fps).</text>',
                 '</g><defs>']
        for i, p in enumerate(references):
            parts.append(f'<image id="ref{i}" width="1920" height="1200" href="{uri(p, "image/jpeg")}"/>')
        parts.append('</defs>')
        for row, ms in enumerate(times):
            y = 50+row*264
            for x in (0, 320, 640, 960):
                parts.append(f'<text x="{x+10}" y="{y+17}" fill="white" font-family="sans-serif" font-size="14">t = {ms/1000:.1f} s</text>')
            for x, ref, start in ((0, 0, 19), (960, 1, 21)):
                index = start+ms//500
                if index < 30:
                    sx, sy = index % 6*320, index//6*240
                    parts.append(f'<svg x="{x}" y="{y+24}" width="320" height="240" viewBox="{sx} {sy} 320 240"><use href="#ref{ref}"/></svg>')
            for x, directory in ((320, before), (640, after)):
                parts.append(f'<image x="{x}" y="{y+24}" width="320" height="240" href="{uri(directory/f"pending-{ms}.png")}"/>')
        parts.append('</svg>')
        (out/name).write_text('\n'.join(parts)+'\n', encoding='utf-8')

    montage('comparison.svg', [0, 1000, 2000, 3000, 4000, 5000])
    montage('comparison-full.svg', list(range(0, 5500, 500)))
    result = {'level': 'MM_CLIMB_002', 'alignment': 'original 13.5s / 83.5s; clone Pending t=0; approximate',
              'references': {str(p.relative_to(root)): digest(p) for p in references},
              'simulation_sha256': digest(before/'simulation.txt'), 'save_sha256': digest(before/'SavedGame.dat'),
              'summaries': summaries, 'images': records}
    (out/'comparison.json').write_text(json.dumps(result, indent=2)+'\n', encoding='utf-8')
    print(f'{len(records)} image pairs; simulation and persistence identical; side-by-side reference/clone sequences retained')


if __name__ == '__main__':
    main(Path(sys.argv[1]).resolve())
