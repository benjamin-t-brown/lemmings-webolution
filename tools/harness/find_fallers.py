"""Find live lemmings by looking for the one thing scenery never does: falling.

Earlier attempts picked lemmings out of memory by "an f64 triple on the tube
that moves smoothly" (`live_terrain_check`). On a level whose tube is turning
that selects hundreds of transformed surface vertices instead -- they move
smoothly, at constant height, at exactly the radius a lemming would have.

Height is the discriminator. The engine's transformed geometry keeps its y,
and the tube's own spin is a rotation about y, so nothing in the scenery
changes height. A lemming dropping out of the entrance changes height fast
and monotonically. So: sample every f64 in the application heap that is
inside the tube's 0..96 height band, several times in quick succession, and
keep the ones that fall every single time.

Run it immediately after entering a level, while lemmings are still being
released, or there will be nothing in the air to find.
"""
from __future__ import annotations

import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from live_terrain_check import APP_HEAP_LIMIT  # noqa: E402
from lrstate import attach, read_mem, ui_state  # noqa: E402
from memdump import k32  # noqa: E402

try:
    from find_level_data import regions
except ImportError:                                    # pragma: no cover
    from diffscan import regions                       # type: ignore


def heap_regions(h):
    out = []
    for item in regions(h):
        base, size = item[0], item[1]
        ok = item[2] if len(item) > 2 else True
        if ok and base < APP_HEAP_LIMIT and size <= 64 * 1024 * 1024:
            out.append((base, size))
    return out


def shot(h, regs):
    return {b: read_mem(h, b, s) for b, s in regs}


def wait_for_release(h, n=1, timeout=60.0):
    """Block until the engine's released counter reaches n.

    A lemming is only in the air for the moment after it is released, so the
    sampling has to be triggered rather than run at an arbitrary time. The
    released counter (0x004CBA78, found by a 0 -> 10 known-value diff) is the
    trigger.
    """
    from lrstate import i32

    t0 = time.time()
    while time.time() - t0 < timeout:
        v = i32(h, 0x004CBA78)
        if v is not None and v >= n:
            return v
        time.sleep(0.002)
    return None


def main() -> int:
    samples = int(sys.argv[1]) if len(sys.argv) > 1 else 5
    interval = float(sys.argv[2]) if len(sys.argv) > 2 else 0.12
    trigger = int(sys.argv[3]) if len(sys.argv) > 3 else 0
    h, _pid = attach()
    if not h:
        print("not running")
        return 1
    if ui_state(h) != 0:
        print("not in a level")
        return 1
    regs = heap_regions(h)
    print(f"{len(regs)} heap regions, "
          f"{sum(s for _, s in regs) / 1e6:.0f} MB", flush=True)
    if trigger:
        print(f"waiting for release #{trigger}", flush=True)
        print("released:", wait_for_release(h, trigger), flush=True)

    shots = []
    for _ in range(samples):
        shots.append(shot(h, regs))
        time.sleep(interval)

    fallers = []
    for base, _size in regs:
        datas = [s.get(base) for s in shots]
        if any(d is None for d in datas):
            continue
        n = min(len(d) for d in datas) // 8
        series = [struct.unpack_from(f"<{n}d", d, 0) for d in datas]
        for i in range(n):
            vals = [s[i] for s in series]
            if not all(0.5 < v < 95.0 for v in vals):
                continue
            steps = [vals[j + 1] - vals[j] for j in range(len(vals) - 1)]
            if all(-6.0 < s < -0.02 for s in steps):
                fallers.append((base + i * 8, vals, steps))

    print(f"\n{len(fallers)} slots falling on every sample\n")
    for addr, vals, steps in fallers[:60]:
        print(f"  {addr:#010x}  " + " ".join(f"{v:7.3f}" for v in vals)
              + "   steps " + " ".join(f"{s:+6.3f}" for s in steps))
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
