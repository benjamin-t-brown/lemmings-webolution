"""Dump a strided record array out of the live game, as floats and ints.

    python dump_records.py ADDR STRIDE COUNT [SAMPLES] [INTERVAL]

Prints each record's fields, and -- when SAMPLES > 1 -- marks the fields that
changed between the first and last sample with a '*', so a record array can
be told apart from static data without guessing at its layout.
"""
from __future__ import annotations

import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lrstate import attach, read_mem  # noqa: E402
from memdump import k32  # noqa: E402


def main() -> int:
    addr = int(sys.argv[1], 0)
    stride = int(sys.argv[2], 0)
    count = int(sys.argv[3], 0)
    samples = int(sys.argv[4]) if len(sys.argv) > 4 else 1
    interval = float(sys.argv[5]) if len(sys.argv) > 5 else 0.5

    h, _pid = attach()
    if not h:
        print("not running")
        return 1
    span = stride * count
    shots = []
    for _ in range(samples):
        shots.append(read_mem(h, addr, span))
        if samples > 1:
            time.sleep(interval)
    first, last = shots[0], shots[-1]
    if not first:
        print("unreadable")
        return 1
    for i in range(count):
        base = i * stride
        print(f"\n-- record {i} @ {addr + base:#010x}")
        for off in range(0, stride, 4):
            o = base + off
            if o + 4 > len(first):
                break
            iv, = struct.unpack_from("<i", first, o)
            fv, = struct.unpack_from("<f", first, o)
            mark = "*" if last and first[o:o + 4] != last[o:o + 4] else " "
            dv = ""
            if o + 8 <= len(first):
                d, = struct.unpack_from("<d", first, o)
                if 1e-6 < abs(d) < 1e6:
                    dv = f"  f64={d:.5f}"
            print(f"  +{off:03x} {mark} i32={iv:<12d} f32={fv:<14.5g}{dv}")
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
