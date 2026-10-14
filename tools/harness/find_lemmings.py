"""Find the original's live lemmings, with the tube rotating.

`live_terrain_check.candidates` collects every f64 triple that lies on the
tube. Most of them are not lemmings: the engine rewrites a transformed vertex
buffer every rendered frame, and the tube spins, so hundreds of surface
vertices move smoothly at constant height -- which is exactly the filter that
module used to call something a walker. On a level where the tube is turning,
that filter selects scenery.

The spin is the way to separate them. Every vertex of the surface shares one
angular velocity, so the median angular velocity over all candidates *is* the
tube's rotation. A lemming is carried by the same rotation plus its own
walking, so it is an outlier in angular velocity; a falling one is also an
outlier in height. This prints both, with the radius, so a lemming can be
told from a vertex before anything is concluded from its position.
"""
from __future__ import annotations

import math
import statistics
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from live_terrain_check import candidates  # noqa: E402
from lrstate import attach, read_mem, ui_state  # noqa: E402
from memdump import k32  # noqa: E402


def read3(h, addr):
    d = read_mem(h, addr, 24)
    return struct.unpack("<3d", d) if d and len(d) == 24 else None


def angle(v) -> float:
    return math.atan2(-v[0], v[2])


def track(h, addrs, samples=4, interval=0.5):
    shots = []
    for _ in range(samples):
        shots.append({a: read3(h, a) for a in addrs})
        time.sleep(interval)
    rows = []
    for a in addrs:
        vs = [s.get(a) for s in shots]
        if any(v is None for v in vs):
            continue
        da = [((angle(vs[i + 1]) - angle(vs[i]) + math.pi) % (2 * math.pi)) - math.pi
              for i in range(samples - 1)]
        dy = [vs[i + 1][1] - vs[i][1] for i in range(samples - 1)]
        r = [math.hypot(v[0], v[2]) for v in vs]
        rows.append((a, statistics.mean(da), statistics.mean(dy),
                     statistics.mean(r), vs[-1]))
    return rows


def main() -> int:
    h, _pid = attach()
    if not h:
        print("not running")
        return 1
    if ui_state(h) != 0:
        print("not in a level")
        return 1
    addrs = candidates(h)
    print(f"{len(addrs)} points on the tube")
    rows = track(h, addrs)
    if not rows:
        print("nothing stable to track")
        return 2
    spin = statistics.median(r[1] for r in rows)
    print(f"tube spin: {spin:+.5f} rad per sample ({len(rows)} tracks)")
    out = [r for r in rows if abs(r[1] - spin) > 0.01 or abs(r[2]) > 0.01]
    print(f"{len(out)} outliers (own motion relative to the tube)\n")
    for a, da, dy, r, v in sorted(out, key=lambda t: -abs(t[1] - spin))[:40]:
        print(f"  {a:#010x} dAng={da - spin:+8.5f} dY={dy:+8.4f} r={r:6.2f}"
              f"  last=({v[0]:8.3f},{v[1]:7.3f},{v[2]:8.3f})")
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
