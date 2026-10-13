"""Check our rasterised terrain mask against the running original.

The open question in PLAN.md 1.6 was to dump the original's in-memory terrain
and correlate it with `lrtool lvlmask`. That framing assumes the engine keeps
an occupancy buffer. It does not -- see `maskscan` below, and PLAN.md -- so
there is nothing to correlate directly.

What the engine *does* expose is where its lemmings actually are. A walker is
standing on terrain by definition, so every walker position read live out of
the original is one sample of "this is solid here". Enough of them, taken
while the original simulates a level we also rasterise, is a direct test of
the mask: if our raster were wrong, lemmings would be walking through cells we
call empty.

Finding them needs no symbols. A lemming is three consecutive f64 in the
engine's heap whose (x, z) lie on the tube (radius 54..62, between the inner
50 and outer 60 skins plus sprite offset) and whose y is inside the tube's
0..96 height. That matches a few hundred addresses, most of them static
geometry, so the set is then polled for a second: a walker moves a little
every frame, smoothly, without changing height. Static geometry does not move
and fallers change height fast, so both drop out.

Run with the original already in a level (see PLAN.md 1.6 for how to get it
there -- use the stock 640x480 exe, not the resolution-patched one).

    python live_terrain_check.py [MM_CLIMB_002]
"""
from __future__ import annotations

import math
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from find_level_data import regions  # noqa: E402
from lrstate import attach, read_mem, ui_state  # noqa: E402
from memdump import k32  # noqa: E402

REPO = Path(__file__).resolve().parents[2]
REF = REPO / "build" / "live_dump" / "ref_climb_mask.bin"
MASK_W, MASK_H = 256, 192
WORLD_H = 96.0
PIXELS_PER_UNIT = 2
APP_HEAP_LIMIT = 0x30000000      # above this is dgVoodoo / d3d / system DLLs


def mask_pixel(x: float, y: float, z: float) -> tuple[int, int]:
    """SurfaceMesh::unrollCylinder + worldToMaskPixel, in Python."""
    a = math.atan2(-x, z) / (2.0 * math.pi)
    if a < 0:
        a += 1.0
    px = int(a * MASK_W) % MASK_W
    py = int((WORLD_H - y) * PIXELS_PER_UNIT)
    return px, max(0, min(MASK_H - 1, py))


def candidates(h) -> list[int]:
    out: list[int] = []
    for base, size, ok in regions(h):
        if not ok or base >= APP_HEAP_LIMIT or size > 64 * 1024 * 1024:
            continue
        d = read_mem(h, base, size)
        if not d:
            continue
        n = len(d) // 8
        if n < 3:
            continue
        vals = struct.unpack_from(f"<{n}d", d, 0)
        for i in range(n - 2):
            y = vals[i + 1]
            if not (1.0 < y < 95.0):
                continue
            x, z = vals[i], vals[i + 2]
            r2 = x * x + z * z
            if 2900.0 < r2 < 3800.0:
                out.append(base + i * 8)
    return out


def walkers(h, addrs, samples: int = 10, interval: float = 0.25):
    hist: dict[int, list] = {a: [] for a in addrs}
    for _ in range(samples):
        for a in addrs:
            d = read_mem(h, a, 24)
            hist[a].append(struct.unpack("<3d", d) if d and len(d) == 24 else None)
        time.sleep(interval)
    found = []
    for a, pts in hist.items():
        if any(p is None for p in pts):
            continue
        steps = [math.dist(pts[i], pts[i + 1]) for i in range(len(pts) - 1)]
        moving = sum(1 for s in steps if 0.005 < s < 2.0)
        heights = [p[1] for p in pts]
        if moving >= samples - 3 and max(steps) < 2.0 and max(heights) - min(heights) < 2.0:
            found.append((a, pts))
    return sorted(found)


def main() -> int:
    h, pid = attach()
    if not h:
        print("the game is not running")
        return 1
    state = ui_state(h)
    print(f"pid {pid}, ui_state {state}")
    if state != 0:
        print("not in a level (ui_state 0 is play); enter one first")
        return 1
    if not REF.exists():
        print(f"missing {REF}; write it with lrtool lvlmask first")
        return 1
    mask = REF.read_bytes()

    addrs = candidates(h)
    print(f"{len(addrs)} points on the tube surface; polling for walkers")
    found = walkers(h, addrs)
    print(f"{len(found)} walker-like tracks\n")

    solid = empty_above = solid_below = 0
    for a, pts in found:
        p = pts[-1]
        px, py = mask_pixel(*p)
        here = mask[py * MASK_W + px]
        above = all(mask[max(0, py - dy) * MASK_W + px] == 0 for dy in (3, 4, 5))
        below = sum(mask[min(MASK_H - 1, py + dy) * MASK_W + px] for dy in (1, 2, 3))
        solid += bool(here)
        empty_above += bool(above)
        solid_below += below >= 2
        print(
            "  %#010x (%8.3f,%7.3f,%8.3f) r=%5.2f -> px (%3d,%3d)  here=%d below3=%d aboveClear=%d"
            % (a, p[0], p[1], p[2], math.hypot(p[0], p[2]), px, py, here, below, above)
        )

    overlay = REPO / "build" / "live_dump" / "live_terrain_check.png"
    try:
        from PIL import Image

        img = Image.new("RGB", (MASK_W, MASK_H))
        img.putdata([(70, 70, 80) if b else (12, 12, 16) for b in mask])
        px_ = img.load()
        for a, pts in found:
            for p in pts:
                x, y = mask_pixel(*p)
                px_[x, y] = (255, 90, 60)
        img.resize((MASK_W * 3, MASK_H * 3), Image.NEAREST).save(overlay)
        print(f"overlay written to {overlay}")
    except Exception as exc:          # PIL is optional for the numbers
        print(f"(no overlay: {exc})")

    n = len(found)
    if n:
        print(
            f"\nstanding on a solid mask cell: {solid}/{n}"
            f"\nsolid continues below:        {solid_below}/{n}"
            f"\nclear above:                  {empty_above}/{n}"
        )
    k32.CloseHandle(h)
    return 0 if n and solid == n else 2


if __name__ == "__main__":
    raise SystemExit(main())
