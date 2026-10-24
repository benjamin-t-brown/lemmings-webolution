"""C2g: score the Out / Saved / timer overlay against the A7 captures.

Offline. Does not launch the game. For each of the nine MM_CLIMB_002 views and MM_DIG_000 it
renders the route with and without `--hud-ticks`, then prints

  * the amended A7 gate from build/tmp/a7p-facilitator/verify.py `wider()` (imported, not
    reimplemented). The amended gate is the caption AND the ten skill-panel bands excluded, i.e.
    wider()'s `numerator_without_skill` / `N_without_skill`, pass iff numerator*50 <= N*765.
    wider()'s own `pass` still counts the skill panel; it is printed as `raw_pass` for reference;
  * for the Out, Saved and timer boxes (the glyph extents the renderer printed as `hud-box:`),
    the summed |dRGB| of render vs the captured baseline.png, before and after, two ways:
      gate   only pixels inside the amended-gate mask (baseline != no-cell-routes, caption
             excluded). The HUD sits over sky and floor, so this is mostly empty.
      all    every pixel in the box. NOT the gate; a supplementary measure.

Run from the repo root with the normal Python (PIL needed), on the machine that has the build:
    python tools/harness/score_c2_hud.py --exe build/c2g/webolution.exe \
        --out build/tmp/c2g-counters-timer-render --out-ref 4
"""

import argparse
import importlib.util
import json
import os
import re
import subprocess
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "build/tmp/a7p-facilitator"))
_spec = importlib.util.spec_from_file_location("a7p_verify", ROOT / "build/tmp/a7p-facilitator/verify.py")
verify = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(verify)

FIX = ROOT / "tools/harness/fixtures/camera_a1"
ZOOM = ROOT / "build/tmp/a1-zoom-elevation"
WORK = verify.WORK
VIEWS = [
    ("initial", "MM_CLIMB_002", WORK / "initial", FIX / "camera-fixture-params.json"),
    ("canonical", "MM_CLIMB_002", WORK / "canonical", ZOOM / "params-canonical.json"),
    ("mouse", "MM_CLIMB_002", WORK / "mouse", FIX / "rotation-mouse-params.json"),
    ("right", "MM_CLIMB_002", WORK / "right", FIX / "rotation-key-right-params.json"),
    ("left", "MM_CLIMB_002", WORK / "left", FIX / "rotation-key-left-params.json"),
    ("zoom1", "MM_CLIMB_002", WORK / "zoom1", ZOOM / "params-zoom1.json"),
    ("elev_up", "MM_CLIMB_002", WORK / "elev_up", ZOOM / "params-elev_up.json"),
    ("elev_down", "MM_CLIMB_002", WORK / "elev_down", ZOOM / "params-elev_down.json"),
    ("elev0_neg", "MM_CLIMB_002", WORK / "elev0_neg", ZOOM / "params-elev0_neg.json"),
    ("MM_DIG_000", "MM_DIG_000", ROOT / "build/tmp/a7q-second-level-mask/no_camera_input",
     ROOT / "build/tmp/a7q-second-level-mask/params-mm_dig_000.json"),
]
REGIONS = ("out", "saved", "timer")
BOX = re.compile(r"hud-box: (\w+) (-?\d+) (-?\d+) (-?\d+) (-?\d+)")


def render(exe: Path, level: str, params: Path, shot: Path, extra: list[str]) -> dict:
    cmd = [str(exe), "--level", level, "--native", "--camera", str(params), "--geom", "route",
           "--no-injected-walkers", "--shot", str(shot), *extra]
    # The exe needs UCRT64 bin to load; give it only to the child, never to this Python.
    env = dict(os.environ)
    ucrt = Path(os.environ.get("MSYS2_ROOT", r"C:\progs\msys2")) / "ucrt64" / "bin"
    env["PATH"] = str(ucrt) + os.pathsep + env.get("PATH", "")
    done = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, env=env)
    if done.returncode != 0 or not shot.exists():
        raise SystemExit(f"render failed ({done.returncode}): {' '.join(cmd)}\n{done.stderr}")
    boxes = {m.group(1): tuple(int(m.group(i)) for i in range(2, 6)) for m in BOX.finditer(done.stderr)}
    hud = re.search(r"hud: tick=\d+ out=(-?\d+) saved=(-?\d+) target=(-?\d+) seconds=(-?\d+)", done.stderr)
    return {"cmd": " ".join(cmd), "boxes": boxes, "hud": hud.group(0) if hud else None}


def region_error(baseline, suppressed, render_img, box, gate_only: bool) -> int:
    x0, y0, x1, y1 = box
    bp, sp, rp = baseline.load(), suppressed.load(), render_img.load()
    total = 0
    for y in range(max(0, y0), min(baseline.height, y1)):
        for x in range(max(0, x0), min(baseline.width, x1)):
            if gate_only and (verify.in_caption(x, y) or bp[x, y] == sp[x, y]):
                continue
            total += sum(abs(a - b) for a, b in zip(bp[x, y], rp[x, y]))
    return total


def amended(g: dict) -> dict:
    den = g["N_without_skill"] * 255 * 3
    return {"numerator": g["numerator_without_skill"], "denominator": den, "N": g["N_without_skill"],
            "mae": g["numerator_without_skill"] / den, "pass": g["numerator_without_skill"] * 50 <= den,
            "raw_pass": g["pass"]}


def ink(img: Image.Image, box, dx=0, dy=0, thr=200):
    """Set of near-white pixels (all channels >= thr) in the box, shifted by (dx, dy)."""
    x0, y0, x1, y1 = box
    px = img.load()
    return {(x + dx, y + dy) for y in range(max(0, y0 - 6), min(img.height, y1 + 6))
            for x in range(max(0, x0 - 6), min(img.width, x1 + 6)) if min(px[x, y]) >= thr}


def ink_shift(base: Image.Image, shot: Image.Image, box, reach=4):
    """Diagnostic only: overlap (intersection / union) of near-white pixels, baseline vs overlay
    shifted by (dx, dy). Reports (0,0) and the best shift; nothing is tuned from it."""
    b = ink(base, box)
    best = None
    at0 = None
    for dy in range(-reach, reach + 1):
        for dx in range(-reach, reach + 1):
            o = ink(shot, box, dx, dy)
            iou = len(b & o) / max(1, len(b | o))
            if dx == 0 and dy == 0:
                at0 = iou
            if best is None or iou > best[0]:
                best = (iou, dx, dy)
    return {"iou_at_0": round(at0, 4), "best_iou": round(best[0], 4), "best_dx": best[1], "best_dy": best[2]}


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", type=Path, default=ROOT / "build/c2g/webolution.exe")
    ap.add_argument("--out", type=Path, default=ROOT / "build/tmp/c2g-counters-timer-render")
    ap.add_argument("--ticks", type=int, default=70, help="sim ticks behind the HUD (14 s at 5/s)")
    ap.add_argument("--out-ref", type=int, default=None,
                    help="MEASURED-FROM-REFERENCE Out digit; adds a variant with the sim's Out replaced")
    ap.add_argument("--colour", default="0,0,0", help="sensitivity colour for Out and the timer")
    ap.add_argument("--json", type=Path, default=None)
    args = ap.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)

    variants = [("after", ["--hud-ticks", str(args.ticks)]),
                ("after_colour", ["--hud-ticks", str(args.ticks), "--hud-colour", args.colour])]
    if args.out_ref is not None:
        variants.insert(1, ("after_out_ref", ["--hud-ticks", str(args.ticks), "--hud-out", str(args.out_ref)]))

    rows = []
    for name, level, folder, params in VIEWS:
        base = Image.open(folder / "baseline.png").convert("RGB")
        supp = Image.open(folder / "no-cell-routes.png").convert("RGB")
        shots = {"before": render(args.exe, level, params, args.out / f"{name}-before.png", [])}
        for vname, extra in variants:
            shots[vname] = render(args.exe, level, params, args.out / f"{name}-{vname}.png", extra)
        boxes = shots["after"]["boxes"]
        imgs = {k: Image.open(args.out / f"{name}-{k}.png").convert("RGB") for k in shots}
        row = {"view": name, "level": level, "hud": shots["after"]["hud"], "boxes": boxes}
        stored = folder / "render.png"
        if stored.exists():
            row["before_equals_stored_render"] = (
                imgs["before"].tobytes() == Image.open(stored).convert("RGB").tobytes())
        for k, img in imgs.items():
            row[f"gate_{k}"] = verify.wider(base, supp, img)
            row[f"amended_{k}"] = amended(row[f"gate_{k}"])
        row["ink_shift"] = {region: ink_shift(base, imgs["after"], boxes[region]) for region in REGIONS}
        if args.out_ref is not None:
            row["ink_shift_out_ref"] = ink_shift(base, imgs["after_out_ref"], boxes["out"])
        row["regions"] = {}
        for region in REGIONS:
            row["regions"][region] = {
                k: {"gate": region_error(base, supp, img, boxes[region], True),
                    "all": region_error(base, supp, img, boxes[region], False)}
                for k, img in imgs.items()}
        rows.append(row)

    keys = ["before"] + [v[0] for v in variants]
    print("amended gate (caption + skill panel excluded): numerator before -> after / denominator")
    for r in rows:
        g = r["amended_before"]
        after = r["amended_after"]
        ok = all(r[f"amended_{k}"]["pass"] for k in keys)
        print(f"{r['view']:<11} {g['numerator']:>8} -> {after['numerator']:>8} / {after['denominator']}"
              f"  N={after['N']}  mae {g['mae']:.6f} -> {after['mae']:.6f}  pass(all variants)={ok}"
              f"  raw_pass(after)={after['raw_pass']}")
    print("\nink overlap (near-white, baseline vs overlay), at shift 0 and best shift within +-4")
    for r in rows:
        print(f"{r['view']:<11} " + "  ".join(
            f"{k}: {v['iou_at_0']:.3f} (best {v['best_iou']:.3f} at {v['best_dx']},{v['best_dy']})"
            for k, v in r["ink_shift"].items()))
    for mode in ("gate", "all"):
        print(f"\nregion numerator, {mode} pixels  (columns: {' | '.join(keys)})")
        for r in rows:
            for region in REGIONS:
                vals = [r["regions"][region][k][mode] for k in keys]
                print(f"{r['view']:<11} {region:<6} {r['boxes'][region]}  " + " | ".join(f"{v:>7}" for v in vals))
    if args.json:
        args.json.write_text(json.dumps(rows, indent=2) + "\n", encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
