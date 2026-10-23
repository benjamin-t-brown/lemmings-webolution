#!/usr/bin/env python3
"""B1 main-menu full-frame scorer: A4d fixed changed-pixel method on the whole 640x480 frame.

Same metric as tools/harness/score_a4_render.py (normalised mean absolute RGB error), but the
ROI is the full frame and the gate is the PLAN.md B1 row: <= 0.02. Also reports the integer
numerator and the changed-pixel count so a second scorer can agree to the pixel.
Usage: score_b1_menu.py <rendered.png> <reference.png> [--diff out.png]
"""
from __future__ import annotations

import json
import sys

from PIL import Image, ImageChops

THRESHOLD = 0.02
SIZE = (640, 480)


def score(candidate: Image.Image, reference: Image.Image) -> dict:
    if candidate.size != SIZE or reference.size != SIZE:
        raise SystemExit(f"expected {SIZE}, got {candidate.size} and {reference.size}")
    diff = ImageChops.difference(reference.convert("RGB"), candidate.convert("RGB"))
    numerator = sum((i % 256) * n for i, n in enumerate(diff.histogram()))
    denominator = 255 * 3 * SIZE[0] * SIZE[1]
    changed = sum(1 for px in diff.getdata() if px != (0, 0, 0))
    mae = numerator / denominator
    return {"numerator": numerator, "denominator": denominator, "mae": mae,
            "changed_pixels": changed, "threshold": THRESHOLD, "passes": mae <= THRESHOLD}, diff


def main() -> None:
    args = sys.argv[1:]
    out = None
    if "--diff" in args:
        i = args.index("--diff")
        out = args[i + 1]
        del args[i:i + 2]
    if len(args) != 2:
        raise SystemExit("usage: score_b1_menu.py <rendered.png> <reference.png> [--diff out.png]")
    report, diff = score(Image.open(args[0]), Image.open(args[1]))
    report.update(candidate=args[0], reference=args[1])
    print(json.dumps(report, indent=2))
    if out:
        diff.point(lambda v: min(255, v * 4)).save(out)


if __name__ == "__main__":
    main()
