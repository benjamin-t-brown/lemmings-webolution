"""How far the live walkers sit from the .LVL lattice, in the file's own frame.

check_mask.py compared those tracks to the rasterised occupancy mask and the
feet missed it. Shifting the mask until the feet landed was underdetermined.
This asks the question the mask cannot: using the same unroll as the loader
(atan2(-x, z), no added yaw), how close is each sample to

  * a surface vertex,
  * a vertex on the same ring (height within 1 world unit),
  * an edge rasteriseSurface would actually stroke.

A small distance means the lattice is the floor and the raster dropped it.
A large distance means the billboard is not in the file's frame, or the
lattice is not what they walk on.

    python nearest_lattice.py [tracks.json]
"""
from __future__ import annotations

import json
import math
import statistics
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "proto"))
from box import Box  # noqa: E402

GAME = Path(r"C:\progs\Lemmings Revolution")
TRACKS = ROOT / "build" / "live_dump" / "live_tracks.json"
MASK = ROOT / "build" / "live_dump" / "ref_climb_mask.bin"
MASK_W = 256
MASK_H = 192
PIXELS = 2
WORLD_H = 96.0
EMPTY = 0xFFFFFFFF
THICK = 10.0
RING = 16.0


def wrap(d: float) -> float:
    while d > MASK_W / 2:
        d -= MASK_W
    while d < -MASK_W / 2:
        d += MASK_W
    return d


def column_of(x: float, z: float) -> float:
    a = math.atan2(-x, z) / (2.0 * math.pi)
    if a < 0:
        a += 1.0
    return a * MASK_W


def u32(d: bytes, o: int) -> int:
    return struct.unpack_from("<I", d, o)[0]


def f64(d: bytes, o: int) -> float:
    return struct.unpack_from("<d", d, o)[0]


def load_level() -> bytes:
    base = Box(str(GAME / "lemmings.box"))
    patch = Box(str(GAME / "patch.box"))
    name = next(n for n in base.names if "MM_CLIMB_002" in n.upper() and n.upper().endswith(".LVL"))
    data = base.read(name)
    if name.upper() in patch.index:
        data = patch.read(name)
    print(f"level {name}  {len(data)} bytes")
    return data


def find_mesh(data: bytes):
    """The same signature scan as Cylinder.cpp findCylinderMesh."""
    n = len(data)
    at = 16
    while at + 24 <= n:
        x, y, z = struct.unpack_from("<3d", data, at)
        if x == 0.0 and y == 0.0 and 10.0 < z < 4096.0:
            ring_count = u32(data, at - 4)
            thickness = f64(data, at - 12)
            if 0 < ring_count <= 64 and math.isfinite(thickness) and thickness > 0:
                count = 0
                while at + (count + 1) * 24 <= n:
                    vx, vy, vz = struct.unpack_from("<3d", data, at + count * 24)
                    r = math.hypot(vx, vz)
                    if not (math.isfinite(r) and math.isfinite(vy) and 5.0 < r < 4096.0
                            and abs(vy) < 4096.0):
                        break
                    count += 1
                if count and count % (ring_count * 2) == 0:
                    per = count // ring_count
                    if per % 2 == 0:
                        verts = [struct.unpack_from("<3d", data, at + i * 24) for i in range(count)]
                        p = at + count * 24
                        if u32(data, p) != ring_count:
                            at += 1
                            continue
                        p += 4 + ring_count * (per // 2) * 8
                        one = u32(data, p)
                        surface_count = u32(data, p + 4)
                        p += 8
                        surface = [struct.unpack_from("<3d", data, p + i * 24)
                                   for i in range(surface_count)]
                        print(f"mesh rings={ring_count} steps={per // 2} "
                              f"thickness={thickness} surface={surface_count} one={one}")
                        return surface, thickness
        at += 1
    raise SystemExit("no cylinder mesh")


def find_grids(data: bytes):
    """The two u32 arrays after the 'Cylinder' chunk, as LevelIndex reads them."""
    title_len = u32(data, 0)
    pos = 4 + title_len
    header = [u32(data, pos + 4 * i) for i in range(12)]
    bytes_a, bytes_b = header[3], header[4]
    cyl = None
    i = 0
    while i + 4 <= len(data):
        ln = u32(data, i)
        if 1 <= ln <= 200 and i + 4 + ln <= len(data):
            s = data[i + 4:i + 4 + ln]
            if all(32 <= c < 127 for c in s) and s == b"Cylinder":
                cyl = i + 4 + ln
                break
            if all(32 <= c < 127 for c in s):
                i += 4 + ln
                continue
        i += 1
    if cyl is None or bytes_a != bytes_b or bytes_a % 4:
        raise SystemExit("grids not found")
    start = None
    for o in range(cyl, min(len(data) - 4, cyl + 128)):
        if u32(data, o) == EMPTY:
            start = o
            break
    if start is None:
        raise SystemExit("no empty cell after Cylinder")
    cells = bytes_a // 4
    g0 = list(struct.unpack_from(f"<{cells}I", data, start))
    g1 = list(struct.unpack_from(f"<{cells}I", data, start + bytes_a))
    print(f"grids {cells} cells, grid0 set {sum(v != EMPTY for v in g0)}, "
          f"grid1 set {sum(v != EMPTY for v in g1)}")
    return g0, g1


def dist(a, b) -> float:
    return math.dist(a, b)


def looks_like(q, ring_face: bool) -> bool:
    ab, dc = dist(q[0], q[1]), dist(q[3], q[2])
    ad, bc = dist(q[0], q[3]), dist(q[1], q[2])
    if abs(ab - THICK) > 1.5 or abs(dc - THICK) > 1.5:
        return False
    if ring_face:
        return abs(ad - RING) <= 1.5 and abs(bc - RING) <= 1.5
    return 8.0 <= ad <= 13.5 and 8.0 <= bc <= 13.5


def collect(surface, grid, ring_stride: int):
    quads = []

    def try_quad(k, step, is_ring):
        if k + step + 1 >= len(grid):
            return
        idx = (grid[k], grid[k + 1], grid[k + step + 1], grid[k + step])
        if any(i == EMPTY or i >= len(surface) for i in idx):
            return
        q = [surface[i] for i in idx]
        if not looks_like(q, is_ring):
            return
        quads.append((q, is_ring))

    for k in range(len(grid) - 17):
        try_quad(k, 16, False)
    if ring_stride > 16:
        for k in range(len(grid) - ring_stride - 1):
            try_quad(k, ring_stride, True)
    return quads


def detect_stride(surface, grid) -> int:
    best_score, best, best_n = 1e99, 0, 0
    for delta in range(48, min(len(grid), 20000), 16):
        ds = []
        for k in range(len(grid) - delta):
            a, b = grid[k], grid[k + delta]
            if a == EMPTY or b == EMPTY or a >= len(surface) or b >= len(surface):
                continue
            ds.append(dist(surface[a], surface[b]))
        if len(ds) < 8:
            continue
        ds.sort()
        med = ds[len(ds) // 2]
        score = abs(med - RING)
        if score < best_score - 1e-6 or (abs(score - best_score) < 1e-6 and len(ds) > best_n):
            best_score, best, best_n = score, delta, len(ds)
    return best if best_score < 2.0 else 0


def outer(a, b):
    return a if math.hypot(a[0], a[2]) >= math.hypot(b[0], b[2]) else b


def stroked_segments(quads):
    """The edges rasteriseSurface strokes: outer vertex of each thickness pair."""
    segs = []
    for q, _ring in quads:
        p0 = outer(q[0], q[1])
        p1 = outer(q[3], q[2])
        segs.append((column_of(p0[0], p0[2]), p0[1], column_of(p1[0], p1[2]), p1[1]))
    return segs


def nearest_vertex(c, y, verts, y_tol=None):
    best = None
    for vc, vy in verts:
        if y_tol is not None and abs(vy - y) > y_tol:
            continue
        dcol = wrap(c - vc)
        dy = y - vy
        px = math.hypot(dcol, dy * PIXELS)
        if best is None or px < best[0]:
            best = (px, dcol, dy)
    return best


def nearest_segment(c, y, segs):
    best = None
    for c0, y0, c1, y1 in segs:
        dc = wrap(c1 - c0)
        dy_s = (y1 - y0) * PIXELS
        # Query relative to c0, in pixel space. Try the query's own wraps by
        # measuring the column residual after projection and wrapping it.
        length2 = dc * dc + dy_s * dy_s
        rel = wrap(c - c0)
        qy = (y - y0) * PIXELS
        if length2 < 1e-18:
            t = 0.0
        else:
            t = max(0.0, min(1.0, (rel * dc + qy * dy_s) / length2))
        dcol = wrap(rel - t * dc)
        dy = y - (y0 + t * (y1 - y0))
        px = math.hypot(dcol, dy * PIXELS)
        if best is None or px < best[0]:
            best = (px, dcol, dy)
    return best


def pct(xs, p):
    if not xs:
        return float("nan")
    xs = sorted(xs)
    return xs[min(len(xs) - 1, int(round((len(xs) - 1) * p)))]


def summarise(label, rows):
    if not rows:
        print(f"  {label}: none")
        return
    px = [r[0] for r in rows]
    dcol = [r[1] for r in rows]
    dy = [abs(r[2]) for r in rows]
    near = sum(1 for a, b in zip(dcol, dy) if abs(a) <= 2.0 and b <= 1.0)
    print(f"  {label}: n={len(rows)}  "
          f"median px {statistics.median(px):.2f}  "
          f"|dcol| med {statistics.median(abs(x) for x in dcol):.2f} "
          f"(p10 {pct([abs(x) for x in dcol], 0.1):.2f}, "
          f"p90 {pct([abs(x) for x in dcol], 0.9):.2f})  "
          f"|dy| med {statistics.median(dy):.3f}  "
          f"within 2 col and 1 unit: {near}/{len(rows)}")
    # Signed column residual, to see a constant frame shift.
    print(f"    signed dcol  med {statistics.median(dcol):+.2f}  "
          f"p10 {pct(dcol, 0.1):+.2f}  p90 {pct(dcol, 0.9):+.2f}")


def main() -> int:
    tracks_path = Path(sys.argv[1]) if len(sys.argv) > 1 else TRACKS
    data = load_level()
    surface, _thick = find_mesh(data)
    g0, g1 = find_grids(data)
    stride = detect_stride(surface, g0) or detect_stride(surface, g1)
    quads = collect(surface, g0, stride) + collect(surface, g1, stride)
    print(f"ring stride {stride}, quads {len(quads)} "
          f"(ring {sum(1 for _q, r in quads if r)}, "
          f"angular {sum(1 for _q, r in quads if not r)})")

    verts = [(column_of(x, z), y) for x, y, z in surface]
    segs = stroked_segments(quads)
    horizontal = []
    vertical = []
    for (q, is_ring) in quads:
        p0 = outer(q[0], q[1])
        p1 = outer(q[3], q[2])
        seg = (column_of(p0[0], p0[2]), p0[1], column_of(p1[0], p1[2]), p1[1])
        (vertical if is_ring else horizontal).append(seg)
    ring32 = [(c, y) for c, y in verts if abs(y - 32.0) < 1.0]
    print(f"surface vertices {len(verts)}, on the y=32 ring {len(ring32)}, "
          f"stroked segments {len(segs)}")

    tracks = json.loads(tracks_path.read_text())["tracks"]
    samples = []
    per_col = []
    for t in tracks:
        seen = set()
        for c, y in zip(t["cols"], t["ys"]):
            samples.append((c, y))
            key = int(c) % MASK_W
            if key not in seen:
                seen.add(key)
                per_col.append((c, y))
    print(f"\n{len(tracks)} tracks, {len(samples)} samples, "
          f"{len(per_col)} distinct columns across tracks")
    ys = {round(y, 3) for _c, y in samples}
    print(f"heights present: {sorted(ys)}")

    mask = MASK.read_bytes() if MASK.exists() else None

    def run(title, pts):
        print(f"\n{title}")
        summarise("any surface vertex",
                  [nearest_vertex(c, y, verts) for c, y in pts])
        summarise("vertex on the same ring",
                  [nearest_vertex(c, y, verts, y_tol=1.0) for c, y in pts])
        summarise("stroked edge",
                  [nearest_segment(c, y, segs) for c, y in pts])
        if mask is not None:
            empty, solid = [], []
            for c, y in pts:
                px = int(c) % MASK_W
                py = int((WORLD_H - y) * PIXELS)
                py = max(0, min(MASK_H - 1, py))
                (solid if mask[py * MASK_W + px] else empty).append((c, y))
            print(f"  mask at the foot: solid {len(solid)}, empty {len(empty)}")
            if empty:
                summarise("empty-cell samples vs same-ring vertex",
                          [nearest_vertex(c, y, verts, y_tol=1.0) for c, y in empty])
                summarise("empty-cell samples vs stroked edge",
                          [nearest_segment(c, y, segs) for c, y in empty])

    run("every recorded sample (turnarounds are over-weighted)", samples)
    run("one sample per column per track", per_col)

    # Where on the y=32 ring the geometry actually is, next to the patrol.
    cols = sorted({round(c, 1) for c, _y in ring32})
    print(f"\ny=32 vertex columns ({len(cols)}):")
    if cols:
        # Cluster into runs separated by more than 2 columns.
        runs = [[cols[0]]]
        for c in cols[1:]:
            if c - runs[-1][-1] > 2.5:
                runs.append([c])
            else:
                runs[-1].append(c)
        print("  " + "  ".join(f"{r[0]:.0f}..{r[-1]:.0f} ({len(r)})" for r in runs))

    def spans(label, edges, y_lo, y_hi):
        hits = []
        for c0, y0, c1, y1 in edges:
            if max(y0, y1) < y_lo or min(y0, y1) > y_hi:
                continue
            hits.append((min(c0, c1), max(c0, c1), y0, y1, wrap(c1 - c0)))
        print(f"\n{label}: {len(hits)} edges meeting y {y_lo:.0f}..{y_hi:.0f}")
        for c0, c1, y0, y1, dc in sorted(hits, key=lambda h: h[0]):
            if 100 <= c0 <= 190 or 100 <= c1 <= 190:
                print(f"  col {c0:7.2f}..{c1:7.2f}  dcol {dc:+6.2f}  "
                      f"y {y0:.2f} -> {y1:.2f}")

    spans("horizontal strokes (angular quads)", horizontal, 31, 33)
    spans("vertical strokes (ring quads)", vertical, 16, 48)

    print("\nvertices with column 110..180 and height 16..48:")
    band = sorted((c, y, math.hypot(x, z))
                  for (x, y, z), (c, _yy) in zip(surface, verts)
                  if 110 <= (c % MASK_W) <= 180 and 16 <= y <= 48)
    for c, y, r in band:
        print(f"  col {c:7.2f}  y {y:6.2f}  r {r:6.2f}")

    # Why the floor strokes skip columns 112..144 even though the vertices
    # are there: are those vertices neighbours in the index lattice, and if
    # so which edge test rejects the quad?
    print("\nindex-lattice neighbours of the y=32 outer vertices, columns 104..176:")
    wanted = []
    for i, ((x, y, z), (c, _y)) in enumerate(zip(surface, verts)):
        if abs(y - 32.0) < 0.5 and 104 <= c <= 176 and math.hypot(x, z) > 55:
            wanted.append(i)
    where = {}
    for name, grid in (("g0", g0), ("g1", g1)):
        for k, idx in enumerate(grid):
            if idx in wanted:
                where.setdefault(idx, []).append((name, k))
    for i in wanted:
        x, y, z = surface[i]
        slots = where.get(i, [])
        print(f"  vtx {i} col {column_of(x, z):.2f} r {math.hypot(x, z):.2f} "
              f"slots {slots}")
        for name, k in slots:
            grid = g0 if name == "g0" else g1
            def desc(slot):
                if slot < 0 or slot >= len(grid) or grid[slot] == EMPTY:
                    return "empty"
                v = surface[grid[slot]]
                return (f"vtx {grid[slot]} col {column_of(v[0], v[2]):.2f} "
                        f"y {v[1]:.2f} r {math.hypot(v[0], v[2]):.2f}")
            print(f"    {name}[{k}]   -1 {desc(k - 1)}   +1 {desc(k + 1)}   "
                  f"+16 {desc(k + 16)}")

    # A walker turns when the column ahead is solid above the foot, so the
    # reversals are walls. The same quad rule that dropped the floor links
    # would drop a vertical link whose thickness partner is empty. Report
    # every populated neighbour of the patrol's outer vertices along the
    # ring stride, and say whether that neighbour clears the face test.
    print("\nring-stride neighbours of the y=32 outer chain, columns 104..176:")
    for i in wanted:
        x, y, z = surface[i]
        for name, k in where.get(i, []):
            grid = g0 if name == "g0" else g1
            for delta, tag in ((-stride, f"-Y({stride})"), (stride, f"+Y({stride})")):
                slot = k + delta
                if slot < 0 or slot >= len(grid) or grid[slot] == EMPTY:
                    other = "empty"
                    partner = ""
                else:
                    v = surface[grid[slot]]
                    other = (f"vtx {grid[slot]} col {column_of(v[0], v[2]):.2f} "
                             f"y {v[1]:.2f} r {math.hypot(v[0], v[2]):.2f}")
                    # The face (k, k+1, k+Y+1, k+Y) needs the thickness
                    # partner on both rings. key-1 held it for the floors.
                    pk = (k - 1) + delta
                    if 0 <= pk < len(grid) and grid[pk] != EMPTY and grid[pk] < len(surface):
                        pv = surface[grid[pk]]
                        partner = (f"  partner col {column_of(pv[0], pv[2]):.2f} "
                                   f"y {pv[1]:.2f} r {math.hypot(pv[0], pv[2]):.2f}")
                    else:
                        partner = "  partner empty"
                print(f"  col {column_of(x, z):6.2f} {name}[{k}] {tag}: {other}{partner}")

    # Outer-only verticals: a y=32 outer vertex whose +Y neighbour is a
    # higher vertex at the same column, and whose thickness partner is
    # empty at both rings. The quad rule drops these. Column 128 is one.
    print("\nouter-only verticals on grid0 (y=32 outer, +Y up, no thickness partner):")
    walls = []
    for k, idx in enumerate(g0):
        if idx == EMPTY or idx >= len(surface):
            continue
        if k + stride >= len(g0) or g0[k + stride] == EMPTY:
            continue
        v = surface[idx]
        up = surface[g0[k + stride]]
        if math.hypot(v[0], v[2]) < 55:
            continue
        if up[1] < v[1] + 8:
            continue
        foot_partner = k > 0 and g0[k - 1] != EMPTY
        top_partner = g0[k - 1 + stride] != EMPTY
        if foot_partner or top_partner:
            continue
        c = column_of(v[0], v[2])
        walls.append((c, v[1], up[1], math.hypot(v[0], v[2]), math.hypot(up[0], up[2])))
    for c, y0, y1, r0, r1 in sorted(walls):
        print(f"  col {c:7.2f}  y {y0:.2f}->{y1:.2f}  r {r0:.2f}->{r1:.2f}")
    print(f"  {len(walls)} such walls")

    def dump_slots(grid, name, lo, hi):
        print(f"\n{name}[{lo}..{hi}):")
        for k in range(lo, hi):
            if grid[k] == EMPTY or grid[k] >= len(surface):
                continue
            v = surface[grid[k]]
            print(f"  [{k}] vtx {grid[k]} col {column_of(v[0], v[2]):7.2f} "
                  f"y {v[1]:6.2f} r {math.hypot(v[0], v[2]):6.2f}")

    dump_slots(g0, "g0", 16700, 16730)
    dump_slots(g0, "g0", 16780, 16810)
    # The same vertex indices in grid1, if the wall lives there.
    ids = {surface.index(v) if False else i
           for i, (x, y, z) in enumerate(surface)
           if 120 <= column_of(x, z) <= 180 and 30 <= y <= 50}
    g1_hits = [(k, idx) for k, idx in enumerate(g1) if idx in ids]
    print(f"\ngrid1 slots pointing at columns 120..180, y 30..50: {len(g1_hits)}")
    for k, idx in g1_hits[:40]:
        v = surface[idx]
        print(f"  g1[{k}] vtx {idx} col {column_of(v[0], v[2]):7.2f} "
              f"y {v[1]:6.2f} r {math.hypot(v[0], v[2]):6.2f}")

    # Angular links the quad rule drops: key+16 joins two vertices, and the
    # thickness partner at key-1 is missing on at least one end so no face
    # is emitted. These are the candidates for the hole under the walkers.
    print("\nouter angular links with a missing thickness partner:")
    dropped = []
    for k, idx in enumerate(g0):
        if idx == EMPTY or k + 16 >= len(g0) or g0[k + 16] == EMPTY:
            continue
        if idx >= len(surface) or g0[k + 16] >= len(surface):
            continue
        a = surface[idx]
        b = surface[g0[k + 16]]
        # Same ring, outer skin, one angular step.
        if abs(a[1] - b[1]) > 1.0:
            continue
        chord = dist(a, b)
        if not (8.0 <= chord <= 13.5):
            continue
        left_inner = k > 0 and g0[k - 1] != EMPTY
        right_inner = g0[k + 15] != EMPTY  # (k+16)-1
        if left_inner and right_inner:
            continue
        ca, cb = column_of(a[0], a[2]), column_of(b[0], b[2])
        ra, rb = math.hypot(a[0], a[2]), math.hypot(b[0], b[2])
        dropped.append((a[1], min(ca, cb), max(ca, cb), left_inner, right_inner,
                        min(ra, rb)))
    by_y = {}
    for y, c0, c1, li, ri, r in sorted(dropped):
        by_y[round(y)] = by_y.get(round(y), 0) + 1
        if abs(y - 32.0) < 0.5 and r > 55:
            print(f"  col {c0:7.2f}..{c1:7.2f}  y {y:.1f}  r {r:.1f}  inners {int(li)}{int(ri)}")
    print(f"  {len(dropped)} dropped links on grid0, by height: "
          + " ".join(f"{y}:{n}" for y, n in sorted(by_y.items())))
    print(f"  of which outer (r>55): {sum(1 for d in dropped if d[5] > 55)}")

    extra = [(c0, y, c1, y) for y, c0, c1, _li, _ri, r in dropped if r > 55]
    if extra:
        print("\nempty-cell samples against dropped links:")
        empty = []
        mask = MASK.read_bytes()
        for c, y in samples:
            px = int(c) % MASK_W
            py = max(0, min(MASK_H - 1, int((WORLD_H - y) * PIXELS)))
            if not mask[py * MASK_W + px]:
                empty.append((c, y))
        summarise("dropped angular link",
                  [nearest_segment(c, y, extra) for c, y in empty])

    # Entrances, as a landmark in the same frame.
    print("\nentrances:")
    i = 0
    while i + 8 <= len(data):
        n = u32(data, i)
        if 12 <= n <= 40 and i + 4 + n <= len(data):
            name = data[i + 4:i + 4 + n]
            if name.startswith(b"Entrance") and name.endswith(b".a3d") and all(32 <= c < 127 for c in name):
                # Object 09 within 80 bytes.
                window = data[i:i + 80]
                at = window.find(b"Object 09")
                if at >= 4 and u32(window, at - 4) == 9 and u32(window, at + 9) == 0:
                    start = i + at + 9 + 4
                    tx, ty, tz = f64(data, start + 9 * 8), f64(data, start + 10 * 8), f64(data, start + 11 * 8)
                    if 20 < math.hypot(tx, tz) < 100:
                        print(f"  {name.decode()}  col {column_of(tx, tz):.2f}  y {ty:.3f}  "
                              f"r {math.hypot(tx, tz):.2f}")
                i += 4 + n
                continue
        i += 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
