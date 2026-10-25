#!/usr/bin/env python3
"""C3a: split the full-frame RGB error across named, disjoint regions.

Offline; never launches the stock game or any process. Reads two PNGs:

  reference  --reference PNG (defaults to historical A6d3 capture for legacy reproduction)
  candidate  a webolution --shot render of MM_CLIMB_002 with the matching camera, default injected walkers,
             --hud-ticks 70 --pause-caption (today's production render with the C2 counters, timer, caption)

The metric is the A6d24 one: sum(|ref RGB - cand RGB|), no crop/mask/alignment, over 640x480, denominator
255*3*640*480 = 235,008,000. Every pixel is given to exactly one region in the stated precedence order, so the
region numerators and pixel counts must sum exactly to the frame's. The script exits non-zero if they do not,
or if any two region rectangles intersect (the partition would then depend on precedence).

Region bounds are fixed in REGIONS below and are not read from the scores. SOURCE says where each came from.

    python tools/harness/score_c3_regions.py [--reference PNG] [--candidate PNG] [--control PNG] [--json OUT]
"""

import argparse
import hashlib
import json
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
W, H = 640, 480
DENOM = 255 * 3 * W * H
GATE_NUM = 4_700_160  # numerator at 0.02 (A6d24)
# Historical C3a default retained for exact legacy reproduction. The active C3 gate
# uses the repair4 capture and must be selected explicitly with --reference.
REF = ROOT / "build/tmp/a6d3-matched-state-bbox/original-live.png"
CANDIDATE = ROOT / "build/tmp/c3a-residual-breakdown/render-hud70-caption.png"
CONTROL = ROOT / "build/tmp/c3a-residual-breakdown/render-control-plain.png"

SKILL_X = (18, 90)
SKILL_Y = [(12, 44), (52, 84), (92, 124), (132, 164), (172, 204), (212, 244), (252, 284), (292, 324),
           (332, 364), (372, 404)]

# (name, [half-open xyxy rects], source), in PRECEDENCE order: a pixel belongs to the first region holding it.
REGIONS = [
    ("skill boxes (10)", [(SKILL_X[0], y0, SKILL_X[1], y1) for y0, y1 in SKILL_Y],
     "READ live at 0x4C9208 stride 0x1B0 (PLAN.md A7 amendment 2026-10-21); same ten rects the A7 gate excludes"),
    ("counters + timer (3)", [(607, 15, 620, 44), (607, 50, 620, 79), (560, 430, 618, 459)],
     "OURS: glyph boxes our renderer printed as `hud-box:` for out/saved/timer (C2g); position constants READ "
     "(620 at 0x44D288, y 15/50, timer 560,430); not measured on the original's pixels"),
    ("caption box", [(277, 422, 363, 454)],
     "MEASURED live (C2h); formula READ (C2f); [277,422,363,454) as in PLAN.md A7"),
    ("actor boxes (3)", [(467, 282, 480, 300), (397, 292, 408, 310), (315, 283, 326, 301)],
     "A6d3 overlay-derived placement boxes, fixed in A6d24; not native controller rectangles"),
    ("tube ROI", [(100, 80, 280, 410)],
     "PLAN.md A6 gate clause 1 / A4d fixed ROI [100,80,280,410] (tools/harness/score_a4_render.py)"),
]
REST = "rest"
FLOOR_RING = ("floor ring", "UNBOUNDED",
              "PLAN.md:314 names a green and purple ring below the tube but gives no rectangle and no "
              "other note in docs/ or PLAN.md bounds one; not invented here; its pixels are inside `rest`")


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def path_label(path: Path) -> str:
    """Repo-relative where possible, otherwise preserve an absolute external path."""
    resolved = path.resolve()
    try:
        return str(resolved.relative_to(ROOT))
    except ValueError:
        return str(resolved)


def load(path: Path) -> bytes:
    with Image.open(path) as im:
        im.load()
        if im.size != (W, H):
            raise SystemExit(f"{path}: expected {W}x{H}, got {im.size}")
        return im.convert("RGB").tobytes()


def rects_intersect(a, b) -> bool:
    return a[0] < b[2] and b[0] < a[2] and a[1] < b[3] and b[1] < a[3]


def check_rect_disjoint() -> list[str]:
    """Every rect against every other rect (inside and across regions); returns violations."""
    flat = [(name, r) for name, rects, _ in REGIONS for r in rects]
    bad = []
    for i, (na, ra) in enumerate(flat):
        if not (0 <= ra[0] < ra[2] <= W and 0 <= ra[1] < ra[3] <= H):
            bad.append(f"{na} {ra} outside frame")
        for nb, rb in flat[i + 1:]:
            if rects_intersect(ra, rb):
                bad.append(f"{na} {ra} intersects {nb} {rb}")
    return bad


def owner_map() -> bytearray:
    """owner[i] = index into REGIONS, or len(REGIONS) for rest. First region in precedence order wins."""
    owner = bytearray([len(REGIONS)]) * (W * H)
    claimed = bytearray(W * H)
    for idx, (_, rects, _) in enumerate(REGIONS):
        for x0, y0, x1, y1 in rects:
            for y in range(y0, y1):
                for x in range(x0, x1):
                    p = y * W + x
                    if not claimed[p]:
                        claimed[p] = 1
                        owner[p] = idx
    return owner


def score(ref: bytes, cand: bytes, owner: bytearray) -> dict:
    n = len(REGIONS) + 1
    num = [0] * n
    px = [0] * n
    black = [0] * n  # numerator at pixels where the candidate is exactly (0,0,0): descriptive only
    for p in range(W * H):
        o = owner[p]
        i = p * 3
        e = abs(ref[i] - cand[i]) + abs(ref[i + 1] - cand[i + 1]) + abs(ref[i + 2] - cand[i + 2])
        num[o] += e
        px[o] += 1
        if cand[i] == 0 and cand[i + 1] == 0 and cand[i + 2] == 0:
            black[o] += e
    names = [r[0] for r in REGIONS] + [REST]
    total_num = sum(abs(a - b) for a, b in zip(ref, cand))  # independent whole-frame loop
    rows = [{"region": names[k], "numerator": num[k], "pixels": px[k], "black_candidate_numerator": black[k]}
            for k in range(n)]
    return {"full_frame_numerator": total_num, "denominator": DENOM, "rows": rows,
            "sum_numerators": sum(num), "sum_pixels": sum(px),
            "partition_exact": sum(num) == total_num and sum(px) == W * H}


def print_table(title: str, path: Path, s: dict) -> None:
    fn = s["full_frame_numerator"]
    print(f"\n== {title}: {path_label(path)}  sha256 {sha(path)}")
    print(f"full-frame numerator {fn} / {DENOM} = {fn / DENOM:.10f}   (gate numerator <= {GATE_NUM}: "
          f"{'pass' if fn <= GATE_NUM else 'FAIL'})")
    print(f"{'region':<22}{'numerator':>12}{'pixels':>9}{'share of frame num':>20}{'num / (765*px)':>16}"
          f"{'of which cand black':>21}")
    for r in s["rows"]:
        share = r["numerator"] / fn if fn else 0.0
        mean = r["numerator"] / (765 * r["pixels"]) if r["pixels"] else 0.0
        print(f"{r['region']:<22}{r['numerator']:>12}{r['pixels']:>9}{share:>19.6%}{mean:>16.6f}"
              f"{r['black_candidate_numerator']:>21}")
    print(f"{'SUM':<22}{s['sum_numerators']:>12}{s['sum_pixels']:>9}")
    print(f"exact-sum check: numerators {s['sum_numerators']} == {fn} and pixels {s['sum_pixels']} == {W * H}: "
          f"{'OK' if s['partition_exact'] else 'FAIL'}")
    print(f"floor ring: {FLOOR_RING[1]} (0 px assigned; in `rest`)")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--reference", type=Path, default=REF,
                    help="reference PNG (default: historical A6d3 capture for legacy reproduction)")
    ap.add_argument("--candidate", type=Path, default=CANDIDATE)
    ap.add_argument("--control", type=Path, default=CONTROL,
                    help="plain render (no --hud-ticks / --pause-caption); skipped if the file is absent")
    ap.add_argument("--json", type=Path)
    args = ap.parse_args()

    if not args.candidate.is_file():
        print(f"candidate missing: {args.candidate}", file=sys.stderr)
        return 2

    bad = check_rect_disjoint()
    print("region bounds (half-open xyxy), precedence order, source:")
    for name, rects, src in REGIONS:
        print(f"  {name}: {rects}\n      source: {src}")
    print(f"  {FLOOR_RING[0]}: {FLOOR_RING[1]}\n      source: {FLOOR_RING[2]}")
    print(f"  {REST}: every pixel not in a region above\n      source: complement")
    print(f"rect-intersection check across all {sum(len(r) for _, r, _ in REGIONS)} rects: "
          f"{'none intersect' if not bad else 'FAIL'}")
    for b in bad:
        print("   ", b)

    ref = load(args.reference)
    reference_hash = sha(args.reference)
    reference_kind = "historical A6d3 default" if args.reference.resolve() == REF.resolve() else "explicit"
    print(f"\nreference ({reference_kind}) {path_label(args.reference)} sha256 {reference_hash}")
    owner = owner_map()
    out = {"reference": path_label(args.reference), "reference_kind": reference_kind,
           "reference_sha256": reference_hash,
           "rect_violations": bad, "runs": {}}
    ok = not bad
    for title, path in (("candidate (hud 70 ticks + caption)", args.candidate), ("control (plain render)", args.control)):
        if not path.exists():
            print(f"\n{title}: {path} absent, skipped")
            continue
        s = score(ref, load(path), owner)
        print_table(title, path, s)
        out["runs"][title] = {"path": path_label(path), "sha256": sha(path), **s}
        ok = ok and s["partition_exact"]
    if args.json:
        args.json.write_text(json.dumps(out, indent=2) + "\n", encoding="utf-8")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
