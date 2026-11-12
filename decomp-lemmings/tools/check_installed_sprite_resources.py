"""Independently compare all installed SPR fields and exact EOF with C++."""

import argparse
from collections import Counter
from pathlib import Path
import struct
import subprocess

from inspect_box import read_index


def hash_bytes(data):
    value = 14695981039346656037
    for byte in data:
        value = ((value ^ byte) * 1099511628211) & 0xffffffffffffffff
    return value


def hash_names(names):
    return hash_bytes(b''.join(name + b'\xff' for name in names))


def parse(data, stats):
    cursor = 0

    def take(size):
        nonlocal cursor
        if size > len(data) - cursor:
            raise ValueError('truncated sprite')
        result = data[cursor:cursor + size]
        cursor += size
        return result

    def u32():
        return struct.unpack('<I', take(4))[0]

    def name(capacity):
        size = struct.unpack('<i', take(4))[0]
        if size > capacity:
            return b''
        return take(max(size, 0))[:capacity - 1].split(b'\0', 1)[0]

    clips = u32()
    stats['clips'] += clips
    fields = [clips]
    for _ in range(clips):
        frames = u32()
        stats['frames'] += frames
        frame_fields = []
        for _ in range(frames):
            pieces = u32()
            stats['pieces'] += pieces
            numeric, names = bytearray(), []
            for _ in range(pieces):
                numeric.extend(take(64))
                names.append(name(260))
            frame_fields.extend((pieces, hash_bytes(numeric), hash_names(names)))
        clip_name = name(128)
        speed = struct.unpack('<Q', take(8))[0]
        fields.extend((frames, hash_names([clip_name]), speed, *frame_fields))
    arrays = u32()
    stats['triple_arrays'] += arrays
    fields.append(arrays)
    for _ in range(arrays):
        kind = u32()
        triples = 16 if kind == 0 else 256 if kind == 1 else 0
        stats['triples'] += triples
        stats[f'triple_type_{kind}'] += 1
        fields.extend((kind, triples, hash_bytes(take(triples * 24))))
    if cursor != len(data):
        raise ValueError(f'trailing sprite data: {cursor}/{len(data)}')
    return fields


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('archives', type=Path, nargs='+')
    args = parser.parse_args()
    expected, stats = [], Counter()
    for archive in args.archives:
        with archive.open('rb') as source:
            for entry in read_index(archive):
                if not entry.name.lower().endswith('.spr'):
                    continue
                source.seek(entry.offset)
                data = source.read(entry.size)
                fields = parse(data, stats)
                expected.append((f'{archive.name}:{entry.name}',
                                 [hash_names([entry.name.encode('latin-1')]), len(data), *fields]))
    if not expected:
        parser.error('no SPR resources found')
    result = subprocess.run([str(args.probe), *map(str, args.archives)],
                            capture_output=True, check=True, text=True)
    rows = result.stdout.splitlines()
    if len(rows) != len(expected):
        raise AssertionError(f'{len(rows)} C++ rows for {len(expected)} SPR resources')
    for row, (name, fields) in zip(rows, expected):
        observed = list(map(int, row.split()))
        if observed != fields:
            first = next((i for i, (a, b) in enumerate(zip(observed, fields)) if a != b),
                         min(len(observed), len(fields)))
            raise AssertionError(f'{name}: field {first} mismatch; '
                                 f'{observed[first:first+3]} != {fields[first:first+3]}')
    print(f'Matched {len(rows)} complete installed SPR resources through exact EOF: {dict(stats)}')


if __name__ == '__main__':
    main()
