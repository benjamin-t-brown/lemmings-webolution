"""C2i: score the pause caption panel on A7 with the caption exclusion removed.

Offline. Does not launch the game. For each of the nine MM_CLIMB_002 views and MM_DIG_000 it
renders the route with the C2g HUD (`--hud-ticks`), without and with `--pause-caption`, then prints

  * the amended A7 gate with the CAPTION EXCLUSION REMOVED and the ten skill-panel bands still
    excluded. This is build/tmp/a7p-facilitator/verify.py `wider()` itself (imported, not
    reimplemented) with only `verify.CAPTION` set to an empty rectangle, so `in_caption` is never
    true; the mask (baseline != no-cell-routes), the error and the skill split are verify's.
    pass iff numerator*50 <= N*765, on `numerator_without_skill` / `N_without_skill`.
  * the region residual inside the caption rectangle [277,422,363,454): the summed |dRGB| of
    render vs the captured baseline.png over ALL pixels, and over the gate-mask pixels only.
  * sensitivity rows (alpha 0 / 255, white fill), reported, never used to choose.

Run from the repo root with the normal Python (PIL needed), on the machine that has the build:
    python tools/harness/score_a7_caption.py --exe build/c2i/webolution.exe \
        --out build/tmp/c2i-caption-render --json build/tmp/c2i-caption-render/score.json
"""

import argparse
import importlib.util
import json
import os
import subprocess
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/harness"))
_spec = importlib.util.spec_from_file_location("a7p_verify", ROOT / "build/tmp/a7p-facilitator/verify.py")
verify = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(verify)
RECT = verify.CAPTION  # [277,422,363,454), kept for the region residual
verify.CAPTION = (0, 0, 0, 0)  # the one change from the A7p gate: no caption exclusion

_hud = importlib.util.spec_from_file_location("score_c2_hud", ROOT / "tools/harness/score_c2_hud.py")
c2g = importlib.util.module_from_spec(_hud)
_hud.loader.exec_module(c2g)
VIEWS = c2g.VIEWS

SENSITIVITY = [
    ("alpha0", ["--caption-alpha", "0"]),
    ("alpha255", ["--caption-alpha", "255"]),
    ("white", ["--caption-fill", "255,255,255"]),
]


def render(exe: Path, level: str, params: Path, shot: Path, extra: list[str], ticks: int) -> None:
    cmd = [str(exe), "--level", level, "--native", "--camera", str(params), "--geom", "route",
           "--no-injected-walkers", "--shot", str(shot), "--hud-ticks", str(ticks), *extra]
    env = dict(os.environ)
    ucrt = Path(os.environ.get("MSYS2_ROOT", r"C:\progs\msys2")) / "ucrt64" / "bin"
    env["PATH"] = str(ucrt) + os.pathsep + env.get("PATH", "")
    done = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, env=env)
    if done.returncode != 0 or not shot.exists():
        raise SystemExit(f"render failed ({done.returncode}): {' '.join(cmd)}\n{done.stderr}")


def region(baseline, suppressed, img, gate_only: bool) -> int:
    x0, y0, x1, y1 = RECT
    bp, sp, rp = baseline.load(), suppressed.load(), img.load()
    total = 0
    for y in range(y0, y1):
        for x in range(x0, x1):
            if gate_only and bp[x, y] == sp[x, y]:
                continue
            total += sum(abs(a - b) for a, b in zip(bp[x, y], rp[x, y]))
    return total


def amended(g: dict) -> dict:
    den = g["N_without_skill"] * 255 * 3
    return {"numerator": g["numerator_without_skill"], "denominator": den, "N": g["N_without_skill"],
            "pass": g["numerator_without_skill"] * 50 <= den}


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", type=Path, default=ROOT / "build/c2i/webolution.exe")
    ap.add_argument("--out", type=Path, default=ROOT / "build/tmp/c2i-caption-render")
    ap.add_argument("--ticks", type=int, default=70)
    ap.add_argument("--json", type=Path, default=None)
    args = ap.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    variants = [("before", []), ("after", ["--pause-caption"])] + [
        (n, ["--pause-caption", *x]) for n, x in SENSITIVITY]

    rows = []
    for name, level, folder, params in VIEWS:
        base = Image.open(folder / "baseline.png").convert("RGB")
        supp = Image.open(folder / "no-cell-routes.png").convert("RGB")
        row = {"view": name}
        for vname, extra in variants:
            shot = args.out / f"{name}-{vname}.png"
            render(args.exe, level, params, shot, extra, args.ticks)
            img = Image.open(shot).convert("RGB")
            g = verify.wider(base, supp, img)
            row[vname] = {"gate": amended(g), "raw_pass": g["pass"],
                          "region_all": region(base, supp, img, False),
                          "region_gate": region(base, supp, img, True)}
        rows.append(row)

    print("amended gate, caption exclusion REMOVED, skill panel excluded: numerator before -> after / denominator")
    for r in rows:
        b, a = r["before"]["gate"], r["after"]["gate"]
        print(f"{r['view']:<11} {b['numerator']:>8} -> {a['numerator']:>8} / {a['denominator']}  N={a['N']}"
              f"  pass before={b['pass']} after={a['pass']}")
    print("\nregion residual in [277,422,363,454): all pixels, before -> after   (gate-mask pixels, before -> after)")
    for r in rows:
        print(f"{r['view']:<11} {r['before']['region_all']:>8} -> {r['after']['region_all']:>8}"
              f"   ({r['before']['region_gate']} -> {r['after']['region_gate']})")
    for vname, _ in SENSITIVITY:
        print(f"\nsensitivity {vname}: gate numerator / pass, region_all")
        for r in rows:
            v = r[vname]
            print(f"{r['view']:<11} {v['gate']['numerator']:>8} pass={v['gate']['pass']}  region_all {v['region_all']}")
    if args.json:
        args.json.write_text(json.dumps(rows, indent=2) + "\n", encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
