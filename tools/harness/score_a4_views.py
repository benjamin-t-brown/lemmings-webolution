#!/usr/bin/env python3
"""Render and score the A4 production frame against the nine retained A2 views.

Runs the built webolution.exe once per view into build/tmp/a4-renderer-core,
then applies the unchanged fixed gate (640x480, ROI [100,80,280,410],
normalized mean absolute RGB error <= 0.02). No captured arena data is used.
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path

from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[2]
EXE = ROOT / "build/cmake/dev-debug/webolution.exe"
FIX = ROOT / "tools/harness/fixtures/camera_a1"
ZOOM = ROOT / "build/tmp/a1-zoom-elevation"
OUT = ROOT / "build/tmp/a4-renderer-core"
ROI = (100, 80, 280, 410)
THRESHOLD = 0.02

VIEWS = {
    "initial": (FIX / "camera-fixture-params.json", FIX / "original-0.png"),
    "mouse": (FIX / "rotation-mouse-params.json", FIX / "original-rotation-mouse-0.png"),
    "right": (FIX / "rotation-key-right-params.json", FIX / "original-rotation-key-right-0.png"),
    "left": (FIX / "rotation-key-left-params.json", FIX / "original-rotation-key-left-0.png"),
    "canonical": (ZOOM / "params-canonical.json", ZOOM / "original-canonical.png"),
    "zoom1": (ZOOM / "params-zoom1.json", ZOOM / "original-zoom1.png"),
    "elev_up": (ZOOM / "params-elev_up.json", ZOOM / "original-elev_up.png"),
    "elev_down": (ZOOM / "params-elev_down.json", ZOOM / "original-elev_down.png"),
    "elev0_neg": (ZOOM / "params-elev0_neg.json", ZOOM / "original-elev0_neg.png"),
}


def error(a: Image.Image, b: Image.Image, roi) -> float:
    d = ImageChops.difference(a.crop(roi), b.crop(roi))
    h = d.histogram()
    return sum((i % 256) * n for i, n in enumerate(h)) / (255 * 3 * d.width * d.height)


def render(camera: Path, shot: Path, extra: list[str]) -> None:
    env = dict(os.environ)
    env.setdefault("WEBOLUTION_GAME_DIR", r"C:/progs/Lemmings Revolution")
    cmd = [str(EXE), "--level", "MM_CLIMB_002", "--native", "--camera", str(camera),
           "--shot", str(shot), *extra]
    subprocess.run(cmd, check=True, env=env, capture_output=True, text=True)
    print("  ran:", " ".join(cmd[1:]), flush=True)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--label", default="wall")
    ap.add_argument("--extra", nargs="*", default=[])
    ap.add_argument("--geom", default="wall")
    ap.add_argument("--tiles", default="flipped")
    ap.add_argument("--order", default="inner-first")
    ap.add_argument("--wall", default="full")
    ap.add_argument("--route", default="",
                    help="single-term route mutation to pass through "
                         "(swap | noinn | centre | texbank)")
    ap.add_argument("--tile-detail", default="hi", choices=("hi", "low"),
                    help="cylinder skin archive variant")
    ap.add_argument("--render", action="store_true")
    args = ap.parse_args()
    extra = ["--geom", args.geom, "--tiles", args.tiles, "--order", args.order,
             "--wall", args.wall, "--tile-detail", args.tile_detail]
    if args.route:
        extra += ["--route", args.route]
    extra += args.extra

    rows = {}
    for name, (camera, original) in VIEWS.items():
        shot = OUT / "renders" / f"{args.label}-{name}.png"
        if args.render:
            render(camera, shot, extra)
        candidate = Image.open(shot).convert("RGB")
        ref = Image.open(original).convert("RGB")
        mae = error(ref, candidate, ROI)
        rows[name] = {"mae": mae, "passes": mae <= THRESHOLD, "png": str(shot.relative_to(ROOT))}
        print(f"{name:10s} {mae:.9f}  {'PASS' if mae <= THRESHOLD else 'fail'}", flush=True)

    passing = [n for n, r in rows.items() if r["passes"]]
    print(f"\nlabel={args.label} extra={args.extra} passing={len(passing)}/{len(rows)}: {passing}")
    report = {"label": args.label, "extra": args.extra, "roi": list(ROI), "threshold": THRESHOLD,
              "views": rows, "passing": passing}
    out = OUT / f"score-{args.label}.json"
    out.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"wrote {out.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
