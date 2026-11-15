"""Bounded source-clone render-demand evidence; no original game execution.

Runs existing drivers synchronously with their original frame counts/assertions.
This is task measurement tooling, not a replacement verifier or scheduler.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import time
import zlib

ROOT = Path(__file__).resolve().parents[1]
RECORDS = ('render-gameplay.txt', 'render-releases.txt', 'render-frame-state.txt',
           'render-actions.txt', 'render-outcomes.txt')


def png(path):
    """Decode only the repository writer's unfiltered RGB/RGBA PNGs."""
    data = path.read_bytes()
    assert data[:8] == b'\x89PNG\r\n\x1a\n'
    pos, compressed = 8, bytearray()
    while pos < len(data):
        size = struct.unpack_from('>I', data, pos)[0]
        tag, body = data[pos + 4:pos + 8], data[pos + 8:pos + 8 + size]
        if tag == b'IHDR':
            w, h, bits, kind, compression, filtering, interlace = struct.unpack('>IIBBBBB', body)
            assert bits == 8 and kind in (2, 6) and compression == filtering == interlace == 0
        if tag == b'IDAT':
            compressed.extend(body)
        pos += size + 12
    channels = 3 if kind == 2 else 4
    raw, rgba = zlib.decompress(compressed), bytearray()
    stride = w * channels + 1
    for y in range(h):
        assert raw[y * stride] == 0
        row = raw[y * stride + 1:(y + 1) * stride]
        if channels == 4:
            rgba.extend(row)
        else:
            for x in range(w):
                rgba.extend(row[x * 3:x * 3 + 3])
                rgba.append(255)
    return w, h, bytes(rgba)


def masks_for(directory):
    result = {}
    for index in directory.rglob('render-inspected.txt'):
        for line in index.read_text().splitlines():
            frame, checksum = map(int, line.split(','))
            mask = (index.parent / f'render-sprites-{frame}.mask').read_bytes()
            # Different test images can share a checksum. Union is safe only
            # within this same level's recorded actual sprite stores.
            key = (index.parent, checksum)
            if key in result:
                mask = bytes(a | b for a, b in zip(result[key], mask))
            result[key] = mask
    return result


def compare(left, right):
    records, images, failures = [], [], []
    for filename in RECORDS:
        a = {p.relative_to(left): p for p in left.rglob(filename)}
        b = {p.relative_to(right): p for p in right.rglob(filename)}
        if a.keys() != b.keys() or not a:
            failures.append(f'record inventory {filename}')
        for key in a.keys() & b.keys():
            same = a[key].read_bytes() == b[key].read_bytes()
            records.append({'path': str(key), 'identical': same,
                            'sha256': hashlib.sha256(a[key].read_bytes()).hexdigest()})
            if not same:
                failures.append(str(key))
    am, bm = masks_for(left), masks_for(right)
    a = {p.relative_to(left): p for p in left.rglob('*.png')}
    b = {p.relative_to(right): p for p in right.rglob('*.png')}
    if a.keys() != b.keys():
        failures.append('PNG inventory')
    for key in sorted(a.keys() & b.keys()):
        if a[key].read_bytes() == b[key].read_bytes():
            images.append({'path': str(key), 'identical': True, 'different_pixels': 0})
            continue
        aw, ah, ap = png(a[key])
        bw, bh, bp = png(b[key])
        assert (aw, ah) == (bw, bh)
        ma = am.get((a[key].parent, zlib.adler32(ap)), b'')
        mb = bm.get((b[key].parent, zlib.adler32(bp)), b'')
        differences = [i for i in range(aw * ah) if ap[i * 4:i * 4 + 4] != bp[i * 4:i * 4 + 4]]
        outside = sum(not ((i < len(ma) and ma[i]) or (i < len(mb) and mb[i])) for i in differences)
        images.append({'path': str(key), 'identical': False, 'different_pixels': len(differences),
                       'outside_actual_sprite_stores': outside})
        if outside:
            failures.append(f'{key}: {outside} unexpected pixels')
    return {'records': records, 'images': images, 'failures': failures}


def specs():
    items = [(f'levelsweep_chunk_{n:02}', 'levelsweep', ['chunk', str(n)]) for n in range(17)]
    items += [('hatch_timing_after', 'hatch_timing_after', ['entry']),
              ('empty_receiver_before', 'empty_receiver_before', []),
              ('empty_receiver_after', 'empty_receiver_after', [])]
    return items


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary-dir', type=Path, default=ROOT / 'build/agent-native/main')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--test', action='append')
    parser.add_argument('--frozen', action='store_true', help='also compare the instrumented authentic entry loop')
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    registry = json.loads((ROOT / 'agent/tools.windows.json').read_text())
    env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy',
               WEBOLUTION_GAME_DIR=str(ROOT / registry['assets']['directory']))
    env['PATH'] = str(Path(registry['tools']['gcc']['path']).parent) + os.pathsep + env['PATH']
    # Read-only snapshots; every application driver uses a private store or no store.
    save_paths = [ROOT / registry['assets']['directory'] / 'SavedGame.dat',
                  Path(os.environ['LOCALAPPDATA']) / 'LemmingsWebolution/SavedGame.dat']
    before_saves = [(p.exists(), p.read_bytes() if p.exists() else None) for p in save_paths]
    summary = {'tests': [], 'saves_unchanged': None}
    for name, driver, arguments in specs():
        if args.test and name not in args.test:
            continue
        row = {'name': name, 'runs': {}}
        modes = ['frozen', 'full', 'skip'] if args.frozen else ['full', 'skip']
        for mode in modes:
            out = args.output / mode / name
            assert not out.exists(), f'refusing to replace evidence: {out}'
            out.mkdir(parents=True)
            exe = args.binary_dir / (('render_skip_reference_' if mode == 'frozen' else 'test_') + driver + '.exe')
            command = [str(exe), *arguments, str(out)]
            child_env = env.copy()
            child_env.pop('WEBOLUTION_TEST_RENDER_ALL', None)
            if mode != 'skip':
                child_env['WEBOLUTION_TEST_RENDER_ALL'] = '1'
            start = time.perf_counter()
            with (out / 'process.log').open('wb') as log:
                completed = subprocess.run(command, env=child_env, cwd=ROOT, stdout=log,
                                           stderr=subprocess.STDOUT, timeout=1500)
            seconds = time.perf_counter() - start
            row['runs'][mode] = {'seconds': seconds, 'exit_code': completed.returncode, 'command': command}
            print(f'{name} {mode}: {seconds:.2f}s exit={completed.returncode}', flush=True)
            assert completed.returncode == 0, f'{name} {mode} failed; see {out}/process.log'
        row['full_vs_skip'] = compare(args.output / 'full' / name, args.output / 'skip' / name)
        if args.frozen:
            row['frozen_vs_full'] = compare(args.output / 'frozen' / name, args.output / 'full' / name)
        summary['tests'].append(row)
        (args.output / 'comparison.json').write_text(json.dumps(summary, indent=2) + '\n')
        for key in ('full_vs_skip', 'frozen_vs_full'):
            assert not row.get(key, {}).get('failures'), row[key]['failures']
    after_saves = [(p.exists(), p.read_bytes() if p.exists() else None) for p in save_paths]
    summary['saves_unchanged'] = before_saves == after_saves
    (args.output / 'comparison.json').write_text(json.dumps(summary, indent=2) + '\n')
    assert summary['saves_unchanged']
    print('All compared state records match; images match outside actual sprite stores; saves unchanged.', flush=True)


if __name__ == '__main__':
    main()
