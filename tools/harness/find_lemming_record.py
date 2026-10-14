"""Catch a lemming's record being born, by snapshotting across one release.

Positions cannot be picked out of this engine by their value alone: the
transformed vertex buffers are full of f64 triples that sit on the tube and
move smoothly (see `find_lemmings`). But a release is an event we can time --
`0x004CBA78` counts lemmings released -- and the record for lemming *n* does
not exist before release *n* and does after.

So: snapshot the application heap while the counter reads n-1, snapshot again
as soon as it reads n, and keep the 8-byte slots whose f64 was implausible
before and is a tube coordinate after. Then sample those a few more times and
keep the ones that keep moving. What survives is state the engine writes per
lemming.
"""
from __future__ import annotations

import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from find_fallers import heap_regions, shot, wait_for_release  # noqa: E402
from lrstate import attach, i32, read_mem, ui_state  # noqa: E402
from memdump import k32  # noqa: E402

RELEASED = 0x004CBA78
LO, HI = 0.2, 95.0


def plausible(v: float) -> bool:
    return LO < v < HI


def main() -> int:
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 1
    h, _pid = attach()
    if not h:
        print("not running")
        return 1
    if ui_state(h) != 0:
        print("not in a level")
        return 1
    regs = heap_regions(h)
    print(f"{len(regs)} regions; released {i32(h, RELEASED)}", flush=True)

    while (i32(h, RELEASED) or 0) >= n:
        print("already past that release")
        return 1
    before = shot(h, regs)
    wait_for_release(h, n)
    after = shot(h, regs)
    print(f"released now {i32(h, RELEASED)}", flush=True)

    born = []
    for base, da in before.items():
        db = after.get(base)
        if not da or not db or len(da) != len(db):
            continue
        m = min(len(da), len(db)) // 8
        va = struct.unpack_from(f"<{m}d", da, 0)
        vb = struct.unpack_from(f"<{m}d", db, 0)
        for i in range(m):
            if va[i] == vb[i]:
                continue
            if plausible(vb[i]) and not plausible(va[i]):
                born.append((base + i * 8, va[i], vb[i]))
    print(f"{len(born)} slots became tube-shaped at the release", flush=True)

    tracks = {a: [v] for a, _o, v in born}
    for _ in range(6):
        time.sleep(0.1)
        for a in list(tracks):
            d = read_mem(h, a, 8)
            tracks[a].append(struct.unpack("<d", d)[0] if d else None)

    live = []
    for a, vs in tracks.items():
        if any(v is None for v in vs):
            continue
        if len(set(vs)) > 2 and all(plausible(v) for v in vs):
            live.append((a, vs))
    print(f"{len(live)} of them keep moving\n")
    for a, vs in sorted(live)[:60]:
        print(f"  {a:#010x}  " + " ".join(f"{v:7.3f}" for v in vs))
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
