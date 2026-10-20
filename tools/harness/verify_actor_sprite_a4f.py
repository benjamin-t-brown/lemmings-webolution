#!/usr/bin/env python3
"""Render and score the A4f actor overlay against a retained native pair.

This is an offline pixel gate, not a native capture tool. It uses the selected
actor's captured semantic frame inputs and the paired actor-free background.
It performs no alignment, rescaling, tint fitting, or image-derived geometry.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import subprocess
from pathlib import Path

from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_GAME_DIR = Path(r"C:\progs\Lemmings Revolution")
UCRT_BIN = Path(r"C:\progs\msys2\ucrt64\bin")


def image_mae(a: Image.Image, b: Image.Image, crop: list[int]) -> dict[str, float | int]:
    diff = ImageChops.difference(a.crop(crop).convert("RGB"), b.crop(crop).convert("RGB"))
    hist = diff.histogram()
    pixels = diff.width * diff.height
    return {
        "mae": sum((i % 256) * n for i, n in enumerate(hist)) / (pixels * 3 * 255),
        "changed_pixels": sum(pixel != (0, 0, 0) for pixel in diff.getdata()),
    }


def rooted(value: str) -> Path:
    path = Path(value)
    return path.resolve() if path.is_absolute() else (ROOT / path).resolve()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--capture", required=True, help="directory containing report.json and paired screenshots")
    parser.add_argument("--label", choices=("five", "eight", "all"), default="all", help="fixture(s) to evaluate")
    parser.add_argument("--output", required=True, help="new or empty directory for candidate images and results")
    parser.add_argument("--game-dir", default=str(DEFAULT_GAME_DIR), help="local retail game directory containing BOX files")
    parser.add_argument(
        "--level",
        default="COMPILED\\MM_CLIMB_002.LVL",
        help="level used for the stored light-map lookup",
    )
    parser.add_argument(
        "--no-light-lookup",
        action="store_true",
        help="draw without level light-map tint (reproduces the pre-A4q candidate)",
    )
    parser.add_argument(
        "--no-alpha-test",
        action="store_true",
        help="draw every texel keeping texture alpha (reproduces the pre-A4u candidate)",
    )
    args = parser.parse_args()

    capture = rooted(args.capture)
    output = rooted(args.output)
    if not (capture / "report.json").is_file():
        parser.error(f"capture report not found: {capture / 'report.json'}")
    if output.exists() and (not output.is_dir() or any(output.iterdir())):
        parser.error(f"output must be new or empty; refusing overwrite: {output}")
    output.mkdir(parents=True, exist_ok=True)

    report = json.loads((capture / "report.json").read_text(encoding="utf-8"))
    pairs = {pair["label"]: pair for pair in report.get("pairs", [])}
    labels = list(pairs) if args.label == "all" else [args.label]
    if not labels or any(label not in pairs for label in labels):
        parser.error(f"requested fixture missing from capture; available: {sorted(pairs)}")

    env = dict(os.environ)
    env["PATH"] = str(UCRT_BIN) + os.pathsep + env.get("PATH", "")
    fixture_exe = ROOT / "build/cmake/dev-debug/actorfixture.exe"
    results: dict[str, dict[str, object]] = {}
    for label in labels:
        pair = pairs[label]
        row = pair["actor_row"]
        position = row["position"]
        overlay_path = output / f"{label}-overlay.png"
        composed_path = output / f"{label}-composed.png"
        command = [
            str(fixture_exe), "--game-dir", args.game_dir, "--out", str(overlay_path),
            "--action-index", str(row["action"]), "--temporal", str(row["temporal"]),
            "--direction", str(row["direction"]), "--mirror", str(row["flag_1b4"]),
            "--scale-delta", "-1", "--x", repr(position[0]), "--y", repr(position[1]),
            "--z", repr(position[2]), "--level", args.level,
        ]
        if args.no_light_lookup:
            command.append("--no-light-lookup")
        if args.no_alpha_test:
            command.append("--no-alpha-test")
        completed = subprocess.run(command, check=True, capture_output=True, text=True, env=env)
        metadata = json.loads(completed.stdout)
        background = Image.open(capture / f"{label}-background-no-actor.png").convert("RGBA")
        native = Image.open(capture / f"{label}-native.png").convert("RGB")
        overlay = Image.open(overlay_path).convert("RGBA")
        if background.size != (640, 480) or native.size != (640, 480) or overlay.size != (640, 480):
            raise RuntimeError(f"{label}: expected fixed 640x480 images; no rescale is performed")
        candidate = Image.alpha_composite(background, overlay).convert("RGB")
        candidate.save(composed_path)
        crop = pair["crop"]
        support = ImageChops.difference(native.crop(crop), background.crop(crop).convert("RGB"))
        support_pixels = [pixel != (0, 0, 0) for pixel in support.getdata()]
        error_pixels = list(ImageChops.difference(candidate.crop(crop), native.crop(crop)).getdata())
        actor_support_error = sum(sum(pixel) for pixel, supported in zip(error_pixels, support_pixels) if supported)
        native_support_count = sum(support_pixels)
        candidate_score = image_mae(candidate, native, crop)
        absent_score = image_mae(background, native, crop)
        alpha_support = sum(value > 0 for value in overlay.getchannel("A").crop(crop).getdata())
        passes = candidate_score["mae"] <= 0.02 and candidate_score["mae"] < absent_score["mae"]
        results[label] = {
            "candidate": candidate_score,
            "absent_actor": absent_score,
            "passes_absent_actor_gate": passes,
            "overlay_nonzero_alpha_pixels": alpha_support,
            "native_actor_support_pixels": native_support_count,
            "support_mae": actor_support_error / (max(1, native_support_count) * 3 * 255),
            "crop": crop,
            "metadata": metadata,
            "overlay_sha256": hashlib.sha256(overlay_path.read_bytes()).hexdigest(),
            "composed_sha256": hashlib.sha256(composed_path.read_bytes()).hexdigest(),
        }
        print(f"{label}: candidate={candidate_score} absent_actor={absent_score} pass={passes}", flush=True)

    result_path = output / "results.json"
    result_path.write_text(json.dumps(results, indent=2) + "\n", encoding="utf-8")
    print(result_path)
    return 0 if all(row["passes_absent_actor_gate"] for row in results.values()) else 1


if __name__ == "__main__":
    raise SystemExit(main())
