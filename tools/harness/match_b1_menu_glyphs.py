#!/usr/bin/env python3
"""Atlas-independent pixel registration diagnostics for the retail menu fonts.

Candidates come from alpha connected supports, never a presumed character grid.
All comparisons keep the scaled crop's physical dimensions on the reference canvas.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
EXPORT = ROOT / "build/tmp/b1e-facilitator/export"
REFERENCE = ROOT / "build/tmp/b1b-reference-screengrab/menu_highlight0.png"
FONT_FILES = {
    "^FONT": "TEXTURES~5C~5EFONT.BMP.png",
    "^FONTH": "TEXTURES~5C~5EFONTH.BMP.png",
    "^FONTP": "TEXTURES~5C~5EFONTP.BMP.png",
    "_FONT": "TEXTURES~5C~5FFONT.BMP.png",
    "_FONTFE": "TEXTURES~5C~5FFONTFE.BMP.png",
    "_FONTSMALL": "TEXTURES~5C~5FFONTSMALL.BMP.png",
}
QUIT = (282, 438, 359, 463)  # measured panel support ROI, half-open


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def components(alpha: Image.Image) -> list[tuple[int, int, int, int]]:
    """8-connected nonzero-alpha bounds; disconnected marks remain candidates."""
    w, h = alpha.size
    pix = alpha.load()
    seen = bytearray(w * h)
    out = []
    for y in range(h):
        for x in range(w):
            i = y * w + x
            if seen[i] or pix[x, y] == 0:
                continue
            seen[i] = 1
            stack = [(x, y)]
            x0 = x1 = x
            y0 = y1 = y
            while stack:
                cx, cy = stack.pop()
                x0, x1 = min(x0, cx), max(x1, cx)
                y0, y1 = min(y0, cy), max(y1, cy)
                for ny in range(max(0, cy - 1), min(h, cy + 2)):
                    for nx in range(max(0, cx - 1), min(w, cx + 2)):
                        ni = ny * w + nx
                        if not seen[ni] and pix[nx, ny] != 0:
                            seen[ni] = 1
                            stack.append((nx, ny))
            out.append((x0, y0, x1 + 1, y1 + 1))
    return out


def target_mask(im: Image.Image) -> Image.Image:
    """Mask foreground by deviation from the measured flat panel RGB."""
    rgb = im.convert("RGB")
    mask = Image.new("L", rgb.size)
    src, dst = rgb.load(), mask.load()
    for y in range(rgb.height):
        for x in range(rgb.width):
            r, g, b = src[x, y]
            # Retain antialiasing as coverage; panel fill is excluded.
            deviation = max(abs(r - 224), abs(g - 10), abs(b))
            # Reject low-level panel variation; retain antialiasing above threshold.
            dst[x, y] = deviation if deviation >= 24 else 0
    return mask


def place_scaled(mask: Image.Image, scale: float, x: int, y: int,
                 canvas_size: tuple[int, int]) -> Image.Image:
    """Scale once, then translate without normalizing to any target box."""
    w, h = mask.size
    size = (max(1, round(w * scale)), max(1, round(h * scale)))
    scaled = mask.resize(size, Image.Resampling.NEAREST)
    canvas = Image.new("L", canvas_size)
    canvas.paste(scaled, (x, y))
    return canvas


def pixel_error(a: Image.Image, b: Image.Image) -> int:
    if a.size != b.size:
        raise ValueError("comparison canvases must be identical")
    return sum(abs(x - y) for x, y in zip(a.getdata(), b.getdata()))


def ordered_assignment(component_indices: list[int]) -> bool:
    """One physical target support per glyph occurrence, in word order."""
    return all(a < b for a, b in zip(component_indices, component_indices[1:])) and len(set(component_indices)) == len(component_indices)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", type=Path, default=ROOT / "build/tmp/b1h-atlas-independent-match/diagnostics.json")
    args = ap.parse_args()
    ref = Image.open(REFERENCE).convert("RGB")
    row = ref.crop(QUIT)
    # Measured target connected boxes from non-panel deviations.
    mask = target_mask(row)
    target = components(mask)
    families = []
    for family, filename in FONT_FILES.items():
        path = EXPORT / filename
        image = Image.open(path).convert("RGBA")
        alpha = image.getchannel("A")
        cs = components(alpha)
        # Group adjacent support components into data-driven crop bounds. No fixed cell
        # dimensions or codepoint labeling is introduced.
        cs.sort(key=lambda b: (b[1], b[0]))
        groups = []
        for a, b in zip(cs, cs[1:]):
            if b[1] - a[3] <= 5 and b[0] >= a[0]:
                groups.append((a[0], a[1], b[2], b[3]))
        families.append({"family": family, "path": str(path.relative_to(ROOT)), "sha256": sha(path),
                         "dimensions": list(image.size), "component_count": len(cs),
                         "components": [list(b) for b in cs], "adjacent_union_candidates": [list(b) for b in groups]})
    result = {"reference": str(REFERENCE.relative_to(ROOT)), "reference_sha256": sha(REFERENCE),
              "target_roi": list(QUIT), "target_components_local": [list(b) for b in target],
              "target_foreground_mask": "absolute RGB deviation from (224,10,0); comparison only",
              "source_families": families,
              "status": "segmentation diagnostics only; no validated ordered glyph registration",
              "full_frame_denominator": 640 * 480 * 3}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
