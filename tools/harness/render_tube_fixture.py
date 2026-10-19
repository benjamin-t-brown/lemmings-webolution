#!/usr/bin/env python3
"""A2 captured-tube fixture with source-derived texture rows and mesh routing.

This is a bounded, offline candidate raster derived from the passing A1 candidate
(`build/tmp/a1-live-arena/render_live.py`) plus the A1 zoom/elevation state
renderer. It is NOT a D3D renderer and it does not run the original game.

Default: the 0x43A4EC walker's INN pass, two OUT side passes, and center
child-short filter, using the true mesh0 AND mesh4 arrays. Diagnostics can
disable INN, draw every INN quad, submit all mesh0 arrays, reverse pass order,
swap texture banks, or revert the source-derived vertical texture row flip.
Retail inputs remain ignored under build/tmp; no original asset is embedded.
This is not the production renderer or a claim of complete scene fidelity.

Projection and depth (READ from the executable, addresses in the round note):
    TL_X = 319 + fx * preX / preZ
    TL_Y = 239 - fy * preY / preZ
    TL_SZ = preZ - context          ; context = 0.0 outer, -0.0003 inner
    TL_RHW = f32(0.25 / f32(TL_SZ)) ; recorded, not used by this raster
The raster interpolates TL_SZ linearly in screen space and keeps the smaller
value (nearer), and interpolates UVs perspective-correctly with 1/preZ.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
HERE = ROOT / "build/tmp/a2-tube"
sys.path.insert(0, str(ROOT / "tools" / "harness"))
from camera_fixture import candidate_camera, grabbed_mode2_yaw  # noqa: E402

FIX = ROOT / "tools" / "harness" / "fixtures" / "camera_a1"
CELL = ROOT / "build/tmp/a1-cell-fixture"
CAPTURE = CELL / "live-capture"
EXTRA = CELL / "facilitator-extra-meshes"
INNER_TEXTURES = CELL / "facilitator-inner-textures"
OUTER_TEXTURES = ROOT / "build/tmp/a1-facilitator"
ARENA_DIR = ROOT / "build/tmp/a1-live-arena"
POOL_SHA = "4e3a52f579c663fc0e97d217d8ea5d780c246f9c6bb33c112534ac6b20c3b630"
MATERIAL_COUNT = 48
DEGENERATE_AREA = 1e-12

PARAM_FILES = {"initial": "camera-fixture-params.json",
               "mouse": "rotation-mouse-params.json",
               "right": "rotation-key-right-params.json",
               "left": "rotation-key-left-params.json"}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_arena():
    arena = (ARENA_DIR / "arena.bin").read_bytes()
    meta = json.loads((ARENA_DIR / "arena.json").read_text())
    assert hashlib.sha256(arena).hexdigest() == meta["sha256"]
    assert meta["sha256"] == "f96bcf7ff21a4ef6f28f0e0f9502444006de4cb5e924f09465f549de20d2ad0e"
    lo = meta["lo"]
    mask = bytearray(len(arena))
    for addr, length in meta["windows"]:
        o = addr - lo
        for j in range(length):
            mask[o + j] = 1
    return arena, mask, lo


def arena_window(arena, mask, lo, addr, n):
    o = addr - lo
    if o < 0 or o + n > len(arena):
        return None
    if not all(mask[o:o + n]):
        return None
    return arena[o:o + n]


def parse_arena_quad(raw: bytes):
    indices = [struct.unpack_from("<I", raw, 0x20 + 0x18 * c)[0] for c in range(4)]
    uvs = [struct.unpack_from("<2d", raw, 0x20 + 0x18 * c + 8) for c in range(4)]
    colour = struct.unpack_from("<4d", raw, 0)
    return indices, uvs, colour


def load_inputs():
    capture = json.loads((CAPTURE / "manifest.json").read_text(encoding="utf-8-sig"))
    extra = json.loads((EXTRA / "manifest.json").read_text(encoding="utf-8-sig"))
    assert capture["loaded_level"].lower() == "compiled\\mm_climb_002.lvl"
    assert capture["camera"]["exact_reference_match"] is True
    assert capture["camera"]["sha256"] == extra["camera_sha256"]
    pool_path = CAPTURE / capture["source_pool"]["file"]
    pool = pool_path.read_bytes()
    assert hashlib.sha256(pool).hexdigest() == POOL_SHA
    assert len(pool) == 897 * 24
    vertices = [struct.unpack_from("<3d", pool, 24 * i) for i in range(897)]
    mat_raw = (CAPTURE / capture["materials"]["file"]).read_bytes()
    assert len(mat_raw) == MATERIAL_COUNT * 24
    materials = [struct.unpack_from("<II2d", mat_raw, 24 * i) for i in range(MATERIAL_COUNT)]
    return capture, extra, vertices, materials, pool_path, CAPTURE / capture["materials"]["file"]


def boundaries(selector: float, extent: float, inner_coefficient: float = 0.7):
    center = (selector - 0.5) * 32.0
    while center < 0:
        center += 32.0
    wide = extent * 32.0
    narrow = extent * inner_coefficient * 32.0
    values = [math.trunc(center - wide), math.trunc(center - narrow), math.trunc(center),
              math.trunc(center + narrow), math.trunc(center + wide)]
    while values[0] < 0:
        values = [v + 32 for v in values]
    while values[0] > 32:
        values = [v - 32 for v in values]
    return values


def inner_index(group: int, entry: int, bands: int = 6) -> int:
    """Loader 0x4608BB: index = (bucket//4) + ((bands-1-band)//2)*8."""
    return (group // 4) + ((bands - 1 - entry) // 2) * 8


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--case", choices=tuple(PARAM_FILES), default=None)
    ap.add_argument("--params", type=Path, default=None,
                    help="camera-state params JSON (as build/tmp/a1-zoom-elevation/params-*.json)")
    ap.add_argument("--label", default="fulltube")
    ap.add_argument("--output-dir", type=Path, default=HERE / "candidates")
    ap.add_argument("--outer-context", type=float, default=0.0)
    ap.add_argument("--inner-context", type=float, default=-0.0003)
    ap.add_argument("--diffuse", choices=("off", "on"), default="on")
    ap.add_argument("--inner-route", choices=("selected", "all", "none"), default="selected")
    ap.add_argument("--outer-route", choices=("walker", "all0"), default="walker")
    ap.add_argument("--order", choices=("inner-first", "outer-first"), default="inner-first")
    ap.add_argument("--bank-swap", choices=("off", "on"), default="off")
    ap.add_argument("--tile-rows", choices=("flipped", "unflipped"), default="flipped")
    ap.add_argument("--depth-test", choices=("sz", "off"), default="sz")
    args = ap.parse_args()

    if (args.case is None) == (args.params is None):
        raise SystemExit("give exactly one of --case or --params")

    capture, extra, vertices, materials, pool_path, mat_path = load_inputs()
    assert len(capture["group_headers"]) == 32
    assert all(g["count"] == 6 for g in capture["group_headers"])

    def material_index_for(entry):
        group, band = entry["group"], entry["entry"]
        derived = inner_index(group, band)
        assert derived == entry["serialized_material_runtime"]
        if args.tile_rows == "unflipped":
            return (group // 4) + (band // 2) * 8
        return derived
    if args.case is not None:
        params_path = FIX / PARAM_FILES[args.case]
    else:
        params_path = args.params.resolve()
    params = json.loads(params_path.read_text())
    width, height = params["viewport"]
    cx, cy = params["center"]
    matrix, eye, fx, fy = candidate_camera(params)
    yaw = grabbed_mode2_yaw(params) if "control_case" in params else float(params["yaw_units"])
    selector = (yaw + float(params["yaw_offset"])) / float(params["yaw_units_per_turn"])
    bounds = boundaries(selector, 0.36)

    image = Image.new("RGB", (width, height), (0, 0, 0))
    depth = [math.inf] * (width * height)

    def project(v):
        x, y, z = v
        px = matrix[0] * x + matrix[3] * y + matrix[6] * z + matrix[9]
        py = matrix[1] * x + matrix[4] * y + matrix[7] * z + matrix[10]
        pz = matrix[2] * x + matrix[5] * y + matrix[8] * z + matrix[11]
        if pz <= 0.0 or not math.isfinite(pz):
            return None
        inv = 1.0 / pz
        return (cx + fx * px * inv, cy - fy * py * inv, pz, inv)

    textures = {}

    def texture_for(material_index: int):
        tile = material_index % 24
        col, row = tile % 8, tile // 8
        is_inner = material_index >= 24
        key = (is_inner, col, row)
        if key not in textures:
            bank = INNER_TEXTURES if is_inner else OUTER_TEXTURES
            textures[key] = Image.open(bank / f"tile-{col}-{row}HI.bmp").convert("RGB")
        return textures[key]

    def triangle(pts, uvs, texture, context, count, modulate=(1.0, 1.0, 1.0)):
        sz = [p[2] - context for p in pts]
        a, b, c = pts
        area = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0])
        if abs(area) < DEGENERATE_AREA:
            return
        xmin = max(0, math.floor(min(p[0] for p in pts)))
        xmax = min(width - 1, math.ceil(max(p[0] for p in pts)))
        ymin = max(0, math.floor(min(p[1] for p in pts)))
        ymax = min(height - 1, math.ceil(max(p[1] for p in pts)))
        tw, th = texture.size
        src = texture.load()
        dst = image.load()
        mr, mg, mb = modulate
        for y in range(ymin, ymax + 1):
            row_at = y * width
            for x in range(xmin, xmax + 1):
                xx, yy = x + 0.5, y + 0.5
                w0 = ((b[0] - xx) * (c[1] - yy) - (b[1] - yy) * (c[0] - xx)) / area
                w1 = ((c[0] - xx) * (a[1] - yy) - (c[1] - yy) * (a[0] - xx)) / area
                w2 = 1.0 - w0 - w1
                if min(w0, w1, w2) < 0.0:
                    continue
                q = w0 * a[3] + w1 * b[3] + w2 * c[3]
                if q <= 0.0:
                    continue
                z = w0 * sz[0] + w1 * sz[1] + w2 * sz[2]
                pos = row_at + x
                if args.depth_test == "sz" and z >= depth[pos]:
                    continue
                u = (w0 * uvs[0][0] * a[3] + w1 * uvs[1][0] * b[3] + w2 * uvs[2][0] * c[3]) / q
                v = (w0 * uvs[0][1] * a[3] + w1 * uvs[1][1] * b[3] + w2 * uvs[2][1] * c[3]) / q
                sx = (u % 1.0) * tw - 0.5
                sy = (v % 1.0) * th - 0.5
                ix, iy = math.floor(sx), math.floor(sy)
                dx, dy = sx - ix, sy - iy
                samples = [src[(ix + i) % tw, (iy + j) % th] for j in (0, 1) for i in (0, 1)]
                weights = ((1 - dx) * (1 - dy), dx * (1 - dy), (1 - dx) * dy, dx * dy)
                dst[x, y] = tuple(
                    min(255, max(0, round(sum(samples[k][ch] * weights[k] for k in range(4)) * m)))
                    for ch, m in enumerate((mr, mg, mb)))
                depth[pos] = z
        count[0] += 1

    def diffuse_modulate(colour):
        if colour is None or args.diffuse == "off":
            return (1.0, 1.0, 1.0)
        r, g, b, a = colour

        def ch(x):
            return min(255, max(0, int(x)))
        return (ch(r) / 255.0, ch(g) / 255.0, ch(b) / 255.0)

    def bank_map(material_index: int) -> int:
        if args.bank_swap == "off":
            return material_index
        return material_index - 24 if material_index >= 24 else material_index + 24

    def draw_quad(indices, uvs, material_index, context, count, colour=None):
        desc = materials[bank_map(material_index)]
        du, dv = desc[2], desc[3]
        pts = []
        tex_uvs = []
        for idx, uv in zip(indices, uvs):
            if idx >= len(vertices) or uv is None:
                return
            p = project(vertices[idx])
            if p is None:
                return
            pts.append(p)
            tex_uvs.append((uv[0] + du, uv[1] + dv))
        texture = texture_for(bank_map(material_index))
        modulate = diffuse_modulate(colour)
        for ids in ((0, 1, 2), (0, 2, 3)):
            triangle([pts[j] for j in ids], [tex_uvs[j] for j in ids], texture, context, count,
                     modulate)

    stats = {"outer_quads_drawn": 0, "outer_triangles": 0,
             "outer_dropped": 0, "inner_quads_drawn": 0, "inner_triangles": 0,
             "inner_flagged_entries": 0, "inner_selected_groups": 0}
    outer_count = [0]
    inner_count = [0]

    def draw_outer():
        arena, arena_mask, arena_lo = load_arena()
        if args.outer_route == "walker":
            a, b, mid, c, d = bounds
            groups = ([(g & 31, "mesh4") for g in range(d-1, mid, -1)]
                      + [(g & 31, "mesh0") for g in range(a, mid)]
                      + [(mid & 31, "center")])
            by_group = {}
            for cell in capture["entries"]:
                by_group.setdefault(cell["group"], []).append(cell)
            submissions = [(e, route) for g, route in groups for e in by_group[g]]
        else:
            submissions = [(e, "mesh0") for e in capture["entries"]]
        center = (selector - 0.5) * 32.0
        while center < 0:
            center += 32.0
        center_angle = center * 8.0
        for entry, route in submissions:
            material_index = material_index_for(entry)
            assert 0 <= material_index < 24, "outer bank material out of 0..23"
            record = bytes.fromhex(entry["record_hex"])
            banks = (0, 4) if route == "center" else ((4,) if route == "mesh4" else (0,))
            quads = []
            for bank in banks:
                base = struct.unpack_from("<I", record, bank)[0]
                child = entry["children_98" if bank == 4 else "children_88"]
                count = child["count"]
                if not base or not count:
                    continue
                header = arena_window(arena, arena_mask, arena_lo, base, 16)
                assert header is not None
                assert struct.unpack_from("<II", header) == (2, count)
                coords_raw = (CAPTURE / child["raw_file"]).read_bytes()
                assert hashlib.sha256(coords_raw).hexdigest() == child["raw_sha256"]
                assert len(coords_raw) == count * 4
                # Side routes traverse the corresponding mesh in reverse.
                indices_q = range(count) if route == "center" else range(count-1, -1, -1)
                for q in indices_q:
                    coord = struct.unpack_from("<h", coords_raw, q*4)[0]
                    if route == "center" and ((bank == 0 and coord >= center_angle)
                                              or (bank == 4 and coord < center_angle)):
                        continue
                    a = base + 0x10 + 0x80 * q
                    raw = arena_window(arena, arena_mask, arena_lo, a, 0x80)
                    if raw is None:
                        stats["outer_dropped"] += 1
                        continue
                    quads.append(parse_arena_quad(raw))
            for indices, uvs, colour in quads:
                before = outer_count[0]
                draw_quad(indices, uvs, material_index, args.outer_context, outer_count, colour)
                if outer_count[0] != before:
                    stats["outer_quads_drawn"] += 1
                    stats["outer_triangles"] += outer_count[0] - before

    def draw_inner():
        if args.inner_route == "none":
            return
        if args.inner_route == "all":
            submissions = capture["entries"]
            stats["inner_selected_groups"] = 32
        else:
            groups = [g & 31 for g in range(bounds[3], bounds[1]-1, -1)]
            stats["inner_selected_groups"] = len(groups)
            by_key = {(e["group"],e["entry"]): e for e in capture["entries"]}
            submissions = [by_key[g,band] for g in groups for band in range(6)]
        for entry in submissions:
            record = bytes.fromhex(entry["record_hex"])
            if record[0xF8] == 0:
                continue
            stats["inner_flagged_entries"] += 1
            indices, uvs = [], []
            for corner in range(4):
                at = 8 + 0x20 + corner * 0x18
                indices.append(struct.unpack_from("<I", record, at)[0])
                uvs.append(struct.unpack_from("<2d", record, at + 8))
            colour = struct.unpack_from("<4d", record, 8)
            material_index = material_index_for(entry) + 24
            before = inner_count[0]
            draw_quad(indices, uvs, material_index, args.inner_context, inner_count, colour)
            if inner_count[0] != before:
                stats["inner_quads_drawn"] += 1
                stats["inner_triangles"] += inner_count[0] - before

    if args.order == "inner-first":
        draw_inner()
        draw_outer()
    else:
        draw_outer()
        draw_inner()

    out_dir = args.output_dir.resolve()
    if (ROOT / "build/tmp").resolve() not in out_dir.parents:
        raise ValueError("candidate output must remain under ignored build/tmp")
    out_dir.mkdir(parents=True, exist_ok=True)
    stem = out_dir / f"sourced-candidate-{args.label}"
    image.save(stem.with_suffix(".png"))
    report = {
        "case": args.case,
        "params": str(params_path.relative_to(ROOT)),
        "params_sha256": sha256(params_path),
        "label": args.label,
        "output": str(stem.with_suffix(".png")),
        "output_sha256": sha256(stem.with_suffix(".png")),
        "selector": selector,
        "bounds_43a4ec": bounds,
        "order": args.order,
        "inner_route": args.inner_route,
        "outer_route": args.outer_route,
        "bank_swap": args.bank_swap,
        "mesh_decode": "true live mesh0/mesh4 headers +0x10, stride 0x80",
        "tile_rows": args.tile_rows,
        "diffuse": args.diffuse,
        "outer_context_58d2b0": args.outer_context,
        "inner_context_58d2b0": args.inner_context,
        "depth_test": args.depth_test,
        "counts": stats,
        "render_calls": {"outer": outer_count[0], "inner": inner_count[0]},
        "inputs": {"pool": str(pool_path.relative_to(ROOT)), "pool_sha256": POOL_SHA,
                   "materials": str(mat_path.relative_to(ROOT)),
                   "materials_sha256": sha256(mat_path)},
        "scope": "captured-geometry offline raster candidate; blend state, device depth state "
                 "and non-tube (level object) geometry are not modelled",
    }
    stem.with_suffix(".json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps({k: report[k] for k in ("label", "order", "inner_route", "bank_swap",
                                             "counts", "render_calls", "output_sha256")}, indent=2))


if __name__ == "__main__":
    main()
