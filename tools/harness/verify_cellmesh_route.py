#!/usr/bin/env python3
"""A4d: re-runnable checks for the tube route the production `--geom route` draws.

Three independent groups, all offline and none of them touching the game:

  1. `--pins` reads the route's own bytes out of the shipped executable: the
     pass-order call sites, the INN gate/inline quad, the OUT bank pointers, the
     centre filter, the material operands and the five stored constants the
     bounds arithmetic uses.

  2. `--oracle` compares the *derived* per-cell rule (the same rule
     `src/assets/CellMesh.cpp` implements) against the retained A1 live arena,
     which is an oracle only -- never an input. It covers the two things A4d
     added: the per-quad child coordinate (entry+0x88 / +0x98, the centre-pass
     filter key) and the entry+8 whole-block inline quad the INN pass submits.

  3. `--pixels` scores one candidate PNG the way the A2 checkpoint did: the
     unchanged fixed ROI, the predeclared cavity and upper-rim rectangles, and
     the deterministic mask of the 179,765 pixels the original's own
     four-cell-route suppression changed.

Retail bytes, the arena and the live captures are all ignored `build/tmp` /
install data; nothing here is a production input. Run with normal Windows
Python (do NOT prepend UCRT64 bin).
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path

from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "harness"))
import generate_cellmesh as g  # noqa: E402

STOCK = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
STOCK_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
ARENA_DIR = ROOT / "build/tmp/a1-live-arena"
CELL_DIR = ROOT / "build/tmp/a1-cell-fixture/live-capture"
DURABLE = ROOT / "build/tmp/a2-tube/live/durable"
ROI = (100, 80, 280, 410)
CAVITY = (280, 80, 430, 290)
UPPER_RIM = (100, 0, 540, 35)
THRESHOLD = 0.02
MASK_PIXELS = 179765

# The route, as read out of the shipped bytes. Every entry is (va, expected
# bytes, what it establishes).
ROUTE_PINS = [
    (0x0043A4FF, "dc25a8034b00", "0x43A4EC bounds: fsub [0x4B03A8] = 0.5 (bucket centre)"),
    (0x0043A505, "dc0d50104b00", "bounds: fmul [0x4B1050] = 32.0"),
    (0x0043A535, "dc0d50104b00", "bounds: extent * 32.0"),
    (0x0043A54F, "dc0d68144b00", "bounds: fmul [0x4B1468] = 0.7 (narrow bracket)"),
    (0x0043A6C6, "dc2560144b00", "INN depth bias: fsub [0x4B1460] = 0.0003"),
    (0x0043A72D, "83c018", "INN material: entry+0xC8 + 0x18"),
    (0x0043A754, "e8a47b0200", "call 0x4622FD (INN pass)"),
    (0x0043A7CA, "8b90c8000000", "OUT right material: entry+0xC8 (no bias)"),
    (0x0043A7F4, "e80b7c0200", "call 0x462404 (OUT right)"),
    (0x0043A87C, "e8247b0200", "call 0x4623A5 (OUT left)"),
    (0x0043A8E0, "dc0df0074b00", "centre filter operand: fmul [0x4B07F0] = 8.0"),
    (0x0043A8FA, "e8667b0200", "call 0x462465 (OUT centre)"),
    (0x0046230B, "8a88f8000000", "INN gate: byte [entry+0xF8]"),
    (0x0046231C, "83c108", "INN quad: entry+8"),
    (0x00462389, "8b82c8000000", "INN resets the material to entry+0xC8 for the OUT passes"),
    (0x004623B1, "833800", "OUT left mesh0 null guard: [entry+0]"),
    (0x004623BE, "81c188000000", "OUT left child array: entry+0x88"),
    (0x00462410, "83780400", "OUT right mesh4 null guard: [entry+4]"),
    (0x0046241E, "81c198000000", "OUT right child array: entry+0x98"),
    (0x004624B4, "0fbf00", "centre filter mesh0: MO16 [child+0], keep < operand"),
    (0x004624C5, "7408", "centre filter mesh0: je skips when C0 clear"),
    (0x00462528, "0fbf10", "centre filter mesh4: MO16 [child], keep >= operand"),
    (0x00462536, "f6c401", "centre filter mesh4: test ah,1 before the keep"),
    (0x00462539, "7508", "centre filter mesh4: jne skips when C0 set"),
]

CONSTANTS = [(0x004B03A8, 0.5), (0x004B1050, 32.0), (0x004B1468, 0.7),
             (0x004B1460, 0.0003), (0x004B07F0, 8.0)]


def read_u32(d: bytes, va: int) -> int:
    o = va - 0x400000
    return struct.unpack_from("<I", d, o)[0]


def audit_pins(exe: Path):
    d = exe.read_bytes()
    digest = hashlib.sha256(d).hexdigest()
    problems = []
    if digest != STOCK_SHA256:
        problems.append(f"exe sha256 {digest} != {STOCK_SHA256}")
    for va, want, label in ROUTE_PINS:
        got = d[va - 0x400000:va - 0x400000 + len(want) // 2].hex()
        if got != want:
            problems.append(f"{va:#010x} {got!r} != {want!r} ({label})")
    for va, want in CONSTANTS:
        got = struct.unpack_from("<d", d, va - 0x400000)[0]
        if got != want:
            problems.append(f"{va:#010x} stored double {got!r} != {want!r}")
    return {"sha256": digest, "pins": len(ROUTE_PINS), "constants": len(CONSTANTS),
            "problems": problems}


def audit_oracle(lvl: Path):
    """The derived rule against the retained arena (oracle only)."""
    height, key_table, records, grid = g.parse_level(lvl)
    lvl_obj, cells = g.generate(height, key_table, records, grid)
    inv = {v: k for k, v in enumerate(key_table) if v != 0xFFFFFFFF}

    def decode(index):
        k = inv[index]
        layer = k % 2
        rest = (k - layer) // 2
        return rest % 0x101, rest // 0x101, layer

    arena = json.loads((ARENA_DIR / "live-arena-manifest.json").read_text(encoding="utf-8-sig"))
    capture = json.loads((CELL_DIR / "manifest.json").read_text(encoding="utf-8-sig"))
    rec = {(e["group"], e["entry"]): e for e in capture["entries"]}

    problems = []
    flag_mismatch = inner_bad = 0
    inner_active = 0
    coord_checked = 0
    for e in arena["entries"]:
        cell_key = (e["group"], e["entry"])
        cell = cells[cell_key]
        if bool(e["inline_flag"]) != bool(cell["wall"]):
            flag_mismatch += 1
        if e["inline_flag"]:
            inner_active += 1
            raw = bytes.fromhex(rec[cell_key]["record_hex"])
            got_i = [struct.unpack_from("<I", raw, 0x28 + 0x18 * c)[0] for c in range(4)]
            got_uv = [struct.unpack_from("<2d", raw, 0x28 + 0x18 * c + 8) for c in range(4)]
            got_col = struct.unpack_from("<4d", raw, 8)
            x0, y0 = cell["x0"], cell["y0"]
            want_i = [lvl_obj.vertex(x0, y0, 0), lvl_obj.vertex(x0, y0 + 16, 0),
                      lvl_obj.vertex(x0 + 8, y0 + 16, 0), lvl_obj.vertex(x0 + 8, y0, 0)]
            u0 = g.u_of(x0)
            u1 = g.u_of(x0 + 8) or 1.0
            v0 = g.v_of(y0)
            v1 = g.v_of(y0 + 16)
            if v1 == 1.0:
                v1 = 0.0
            want_uv = [(u0, v0), (u0, v1), (u1, v1), (u1, v0)]
            if got_i != want_i or got_uv != want_uv or tuple(got_col) != (255.0, 255.0, 255.0, 0.0):
                inner_bad += 1
        for bank, child in (("mesh0", "children_88"), ("mesh4", "children_98")):
            info = rec[cell_key][child]
            if info["count"] == 0:
                continue
            raw = (CELL_DIR / info["raw_file"]).read_bytes()
            if hashlib.sha256(raw).hexdigest() != info["raw_sha256"]:
                problems.append(f"{cell_key} {child}: raw sha mismatch")
            got = [struct.unpack_from("<h", raw, 4 * i)[0] for i in range(len(raw) // 4)]
            gen = [q["coord"] for q in cell[bank]]
            coord_checked += len(gen)
            if got != gen:
                problems.append(f"{cell_key} {bank}: child coords {got[:6]} != {gen[:6]}")
    if flag_mismatch:
        problems.append(f"{flag_mismatch} cells disagree on the wall/inline flag")
    if inner_bad:
        problems.append(f"{inner_bad} inline quads disagree on geometry/UV/colour")
    if inner_active != 98:
        problems.append(f"{inner_active} active inline quads, expected 98")
    return {"level": str(lvl), "inline_quads": inner_active, "coord_values_checked": coord_checked,
            "flag_mismatches": flag_mismatch, "inner_mismatches": inner_bad,
            "problems": problems,
            "scope": "derived rule vs retained arena; the arena is an oracle, never an input"}


def error(a, b, roi=None):
    x = a if roi is None else a.crop(roi)
    y = b if roi is None else b.crop(roi)
    d = ImageChops.difference(x, y)
    h = d.histogram()
    return sum((i % 256) * n for i, n in enumerate(h)) / (255 * 3 * d.width * d.height)


def audit_pixels(candidate: Path, original: Path):
    problems = []
    for p in (DURABLE / "baseline.png", DURABLE / "no-cell-routes.png"):
        if not p.exists():
            problems.append(f"missing retained live capture {p}")
    ref = Image.open(original).convert("RGB")
    cand = Image.open(candidate).convert("RGB")
    out = {"candidate": str(candidate), "candidate_sha256":
           hashlib.sha256(candidate.read_bytes()).hexdigest(),
           "roi": error(ref, cand, ROI), "cavity": error(ref, cand, CAVITY),
           "upper_rim": error(ref, cand, UPPER_RIM), "problems": problems}
    out["roi_passes"] = out["roi"] <= THRESHOLD
    if not problems:
        base = Image.open(DURABLE / "baseline.png").convert("RGB")
        now = Image.open(DURABLE / "no-cell-routes.png").convert("RGB")
        r, gg, b = ImageChops.difference(base, now).split()
        mask = ImageChops.lighter(ImageChops.lighter(r, gg), b).point(lambda n: 255 if n else 0)
        count = mask.histogram()[255]
        if count != MASK_PIXELS:
            out["problems"].append(f"changed-pixel mask is {count}, expected {MASK_PIXELS}")
        h = ImageChops.difference(base, cand).histogram(mask=mask)
        out["changed_pixel_mask"] = count
        out["changed_pixel_score"] = sum((i % 256) * n for i, n in enumerate(h)) / (255 * 3 * count)
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--exe", type=Path, default=STOCK)
    ap.add_argument("--lvl", type=Path, default=g.DEFAULT_LVL)
    ap.add_argument("--candidate", type=Path,
                    default=ROOT / "build/tmp/a4-renderer-core/renders/route-hi-initial.png")
    ap.add_argument("--original", type=Path,
                    default=ROOT / "tools/harness/fixtures/camera_a1/original-0.png")
    ap.add_argument("--pins", action="store_true")
    ap.add_argument("--oracle", action="store_true")
    ap.add_argument("--pixels", action="store_true")
    args = ap.parse_args()
    if not (args.pins or args.oracle or args.pixels):
        args.pins = args.oracle = args.pixels = True

    report = {}
    rc = 0
    if args.pins:
        report["pins"] = audit_pins(args.exe)
        print(f"-- route pins --\n  {report['pins']['pins']} instructions, "
              f"{report['pins']['constants']} stored doubles")
        for p in report["pins"]["problems"]:
            print("  BAD", p)
        rc |= 1 if report["pins"]["problems"] else 0
    if args.oracle:
        report["oracle"] = audit_oracle(args.lvl)
        o = report["oracle"]
        print(f"-- derived rule vs the retained arena (oracle) --\n"
              f"  inline quads {o['inline_quads']}  child coords checked {o['coord_values_checked']}"
              f"  flag mismatches {o['flag_mismatches']}  inner mismatches {o['inner_mismatches']}")
        for p in o["problems"][:8]:
            print("  BAD", p)
        rc |= 1 if o["problems"] else 0
    if args.pixels:
        if not args.candidate.exists():
            print(f"-- pixels --\n  no candidate at {args.candidate}; render it first")
            rc |= 1
        else:
            report["pixels"] = audit_pixels(args.candidate, args.original)
            p = report["pixels"]
            print(f"-- pixels (unchanged gate: ROI {ROI}, <= {THRESHOLD}) --\n"
                  f"  roi {p['roi']:.9f} {'PASS' if p['roi_passes'] else 'fail'}\n"
                  f"  cavity {p['cavity']:.9f} (fails, retained)\n"
                  f"  upper rim {p['upper_rim']:.9f} (fails, retained)")
            if "changed_pixel_score" in p:
                print(f"  changed-pixel mask {p['changed_pixel_mask']} px: "
                      f"{p['changed_pixel_score']:.9f} (fails, retained)")
            for q in p["problems"]:
                print("  BAD", q)
            rc |= 1 if p["problems"] else 0
    (ROOT / "build/tmp/a4-cellmesh-route/verify-route.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print("cellmesh-route verifier:", "OK" if rc == 0 else "FAILED")
    return rc


if __name__ == "__main__":
    sys.exit(main())
