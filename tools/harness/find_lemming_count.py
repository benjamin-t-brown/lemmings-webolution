"""Find the engine's lemming bookkeeping by a known-value diff: 0 -> 10.

MM_CLIMB_002 releases exactly 10 lemmings (LEVELMAP.DAT: limit 300 s, save 1,
total 10). So at the start of the level something in memory holds 0 and a
minute later it holds 10, and almost nothing else in 76 MB does that. The
slots that make that exact transition are the released counter, and whatever
array indexes off it is the lemming list.

This is the counterpart to `find_fallers`: positions are hard to pick out of
the engine's transformed vertex buffers, but a small integer with a known
start and end value is not.
"""
from __future__ import annotations

import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from diffscan import snap  # noqa: E402
from lrstate import attach, i32, ui_state  # noqa: E402
from memdump import k32  # noqa: E402

CLOCK = 0x004CBAAC


def transitions(a, b, want_from=(0,), want_to=(10,)):
    out = []
    for base, da in a.items():
        db = b.get(base)
        if not da or not db or len(da) != len(db):
            continue
        for off in range(0, len(da) - 4, 4):
            if da[off:off + 4] == db[off:off + 4]:
                continue
            x, = struct.unpack_from("<i", da, off)
            y, = struct.unpack_from("<i", db, off)
            if x in want_from and y in want_to:
                out.append((base + off, x, y))
    return out


def main() -> int:
    wait = float(sys.argv[1]) if len(sys.argv) > 1 else 90.0
    total = int(sys.argv[2]) if len(sys.argv) > 2 else 10
    h, _pid = attach()
    if not h:
        print("not running")
        return 1
    if ui_state(h) != 0:
        print("not in a level")
        return 1
    print(f"clock {i32(h, CLOCK)}; snapshotting", flush=True)
    a = snap(h)
    print(f"waiting {wait:.0f}s for all {total} to be released", flush=True)
    time.sleep(wait)
    print(f"clock {i32(h, CLOCK)}; snapshotting", flush=True)
    b = snap(h)

    for lo in ((0,), (0, 1), tuple(range(0, 4))):
        hits = transitions(a, b, lo, (total,))
        print(f"\n{lo} -> {total}: {len(hits)} slots")
        for addr, x, y in hits[:40]:
            print(f"  {addr:#010x}  {x} -> {y}")
        if hits:
            break
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
