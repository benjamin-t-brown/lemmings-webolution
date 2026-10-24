"""Score the saved C1t renders inside the read-geometry footprint.

Offline. Does not render, does not start the stock game, and does not modify
the fixture. MAE is the sum of absolute RGB channel differences divided by
pixels * 765, the same definition as tools/harness/score_c1_panel.py.

An empty plates list does not score. Exactly ten plates write
build/tmp/c1w-plate-footprint/score-table.json. The C1u table is not written.

Usage:
    python tools/harness/score_c1_footprint.py
"""
from __future__ import annotations

import json
import math
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = ROOT / "tools/harness/fixtures/c1-footprint/footprint.json"
OUT = ROOT / "build/tmp/c1w-plate-footprint/score-table.json"
INITIAL_REFERENCE = ROOT / "tools/harness/fixtures/camera_a1/original-0.png"
C1T = ROOT / "build/tmp/c1t-q3-panel-render"

VIEWS = (
    {
        "name": "initial",
        "reference": INITIAL_REFERENCE,
        "render": C1T / "initial/render.png",
        "no_panel": C1T / "initial/no-panel.png",
    },
    {
        "name": "a7q_mm_dig_000",
        "reference": None,
        "render": C1T / "a7q_mm_dig_000/render.png",
        "no_panel": C1T / "a7q_mm_dig_000/no-panel.png",
    },
)


def footprint_pixels(fixture: dict, width: int, height: int) -> set[tuple[int, int]]:
    covered: set[tuple[int, int]] = set()
    for key in ("icons", "glyphs", "plates"):
        for rect in fixture[key]:
            x0 = max(0, math.floor(rect["left"]))
            x1 = min(width, math.ceil(rect["right"]))
            y0 = max(0, math.floor(rect["top"]))
            y1 = min(height, math.ceil(rect["bottom"]))
            for y in range(y0, y1):
                for x in range(x0, x1):
                    covered.add((x, y))
    return covered


def channel_mae(reference: Image.Image, candidate: Image.Image, pixels: set[tuple[int, int]]) -> dict:
    if reference.size != candidate.size:
        raise SystemExit(f"size mismatch {reference.size} vs {candidate.size}")
    ref = reference.convert("RGB").load()
    img = candidate.convert("RGB").load()
    numerator = 0
    for x, y in pixels:
        numerator += sum(abs(a - b) for a, b in zip(ref[x, y], img[x, y]))
    count = len(pixels)
    denominator = count * 765
    return {
        "numerator": numerator,
        "pixels": count,
        "denominator": denominator,
        "mae": (numerator / denominator) if count else None,
    }


def unnamed(pixels: int) -> dict:
    return {
        "numerator": None,
        "pixels": pixels,
        "denominator": None,
        "mae": "reference-unnamed",
    }


def main() -> int:
    fixture = json.loads(FIXTURE.read_text(encoding="utf-8"))
    plates = fixture.get("plates")
    if not isinstance(plates, list) or len(plates) == 0:
        raise SystemExit("plates is empty; an empty list does not score")
    if len(plates) != 10:
        raise SystemExit(f"expected the ten C1v plates, found {len(plates)}")
    table: dict = {
        "fixture": FIXTURE.relative_to(ROOT).as_posix(),
        "mae_definition": "sum of absolute RGB channel differences / (pixels * 765)",
        "views": {},
    }
    for view in VIEWS:
        render = Image.open(view["render"])
        no_panel = Image.open(view["no_panel"])
        if render.size != no_panel.size:
            raise SystemExit(f"{view['name']}: render and no-panel sizes differ")
        pixels = footprint_pixels(fixture, render.size[0], render.size[1])
        row = {
            "render_path": view["render"].relative_to(ROOT).as_posix(),
            "no_panel_path": view["no_panel"].relative_to(ROOT).as_posix(),
            "image_size": list(render.size),
            "pixels": len(pixels),
        }
        if view["reference"] is None:
            row["reference"] = "reference-unnamed"
            row["reason"] = (
                "C1t recorded no stock-capture file for MM_DIG_000. The A7q note "
                "does not name baseline.png as the original's screenshot."
            )
            row["render.png"] = unnamed(len(pixels))
            row["no-panel.png"] = unnamed(len(pixels))
        else:
            reference = Image.open(view["reference"])
            row["reference"] = view["reference"].relative_to(ROOT).as_posix()
            row["render.png"] = channel_mae(reference, render, pixels)
            row["no-panel.png"] = channel_mae(reference, no_panel, pixels)
        table["views"][view["name"]] = row

    printed = {
        "initial": {
            "render.png": table["views"]["initial"]["render.png"]["mae"],
            "no-panel.png": table["views"]["initial"]["no-panel.png"]["mae"],
        },
        "a7q_mm_dig_000": {
            "render.png": table["views"]["a7q_mm_dig_000"]["render.png"]["mae"],
            "no-panel.png": table["views"]["a7q_mm_dig_000"]["no-panel.png"]["mae"],
        },
    }
    table["mae"] = printed
    text = json.dumps(table, indent=2) + "\n"
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(text, encoding="utf-8", newline="\n")
    print(json.dumps(printed, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
