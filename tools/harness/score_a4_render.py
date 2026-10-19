#!/usr/bin/env python3
"""Independent fixed-ROI scorer for the A4 production render.

Unchanged A2 gate: 640x480, ROI [100,80,280,410], normalized mean absolute RGB
error <= 0.02. Reads a rendered PNG and a retained original and reports the
score plus the extra A2 diagnostic rectangles. Normal Windows Python + Pillow.
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[2]
ROI = (100, 80, 280, 410)
THRESHOLD = 0.02
EXTRA = {
    "initial": {"cavity": (280, 80, 430, 290), "upper_rim": (100, 0, 540, 35)},
    "zoom1": {"inner_wall": (205, 80, 275, 300)},
}


def error(a: Image.Image, b: Image.Image, roi) -> float:
    d = ImageChops.difference(a.crop(roi), b.crop(roi))
    h = d.histogram()
    return sum((i % 256) * n for i, n in enumerate(h)) / (255 * 3 * d.width * d.height)


def main() -> None:
    if len(sys.argv) < 3:
        raise SystemExit("usage: score_a4_render.py <candidate.png> <original.png> [view]")
    candidate = Image.open(sys.argv[1]).convert("RGB")
    original = Image.open(sys.argv[2]).convert("RGB")
    view = sys.argv[3] if len(sys.argv) > 3 else "initial"
    if candidate.size != (640, 480):
        raise SystemExit(f"candidate is {candidate.size}, expected 640x480")
    mae = error(original, candidate, ROI)
    report = {"candidate": str(sys.argv[1]), "original": str(sys.argv[2]), "view": view,
              "roi": list(ROI), "threshold": THRESHOLD, "mae": mae, "passes": mae <= THRESHOLD,
              "extra": {name: {"roi": list(r), "mae": error(original, candidate, r)}
                        for name, r in EXTRA.get(view, {}).items()}}
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
