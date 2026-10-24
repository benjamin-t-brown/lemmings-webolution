"""C1b acceptance score: skill-only MAE for the ten skill-panel icons.

Entirely offline. Regenerates `render.png` for the two already-accepted
views this round scores against (`initial`/MM_CLIMB_002 from A7p, and the
one A7q/MM_DIG_000 view) using the exact already-recorded camera params and
`--shot` (no live game, no new stock start), then reuses
`build/tmp/a7p-facilitator/verify.py`'s own `wider()`/`in_skill()` -- loaded
directly from that file, not re-typed -- to compute the skill-only
numerator/denominator/MAE against the already-saved `baseline.png` for each
view.

Usage:
    python tools/harness/score_c1_panel.py
"""

from __future__ import annotations

import importlib.util
import json
import subprocess
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
A7P = ROOT / "build/tmp/a7p-unpause-then-masks"
A7Q = ROOT / "build/tmp/a7q-second-level-mask"
RESTORE_C1B_PRE_PANEL = "--restore-c1b" in sys.argv
OUT = ROOT / ("build/tmp/c1b-panel-icon-render" if RESTORE_C1B_PRE_PANEL
              else "build/tmp/c1t-q3-panel-render")
INVOKE_UCRT64 = ROOT / "scripts" / "Invoke-Ucrt64.ps1"


def _load_verify_module():
    """Import build/tmp/a7p-facilitator/verify.py's wider()/in_skill() by
    path, so this script reuses that exact code rather than retyping it."""
    path = ROOT / "build/tmp/a7p-facilitator/verify.py"
    spec = importlib.util.spec_from_file_location("a7p_facilitator_verify", path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


VIEWS = [
    {
        "name": "initial",
        "level": "MM_CLIMB_002",
        "camera": "tools/harness/fixtures/camera_a1/camera-fixture-params.json",
        "extra_args": ["--no-injected-walkers"],
        "baseline_dir": A7P / "initial",
    },
    {
        "name": "a7q_mm_dig_000",
        "level": "MM_DIG_000",
        "camera": "build/tmp/a7q-second-level-mask/params-mm_dig_000.json",
        "extra_args": [],
        "baseline_dir": A7Q / "no_camera_input",
    },
]


def render(view: dict) -> tuple[Path, int, str]:
    out_dir = OUT / view["name"]
    out_dir.mkdir(parents=True, exist_ok=True)
    shot = out_dir / "render.png"
    command_parts = [
        "build/cmake/dev-debug/webolution.exe",
        "--level", view["level"],
        "--native",
        "--camera", view["camera"],
        "--geom", "route",
        *view["extra_args"],
        *([] if RESTORE_C1B_PRE_PANEL else ["--skill-panel"]),
        "--shot", str(shot.relative_to(ROOT).as_posix()),
    ]
    command = " ".join(command_parts)
    completed = subprocess.run(
        ["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
         "-File", str(INVOKE_UCRT64), command],
        cwd=ROOT, capture_output=True, text=True)
    (out_dir / "render-log.txt").write_text(
        f"command: {command}\nexit {completed.returncode}\n{completed.stdout}\n{completed.stderr}\n",
        encoding="utf-8", newline="\n")
    return shot, completed.returncode, command


def main() -> int:
    verify = _load_verify_module()
    results = {}
    for view in VIEWS:
        shot, returncode, command = render(view)
        row: dict = {"command": command, "render_exit": returncode}
        if returncode != 0 or not shot.exists():
            row["error"] = f"render failed, exit {returncode}, see render-log.txt"
            results[view["name"]] = row
            continue
        baseline_dir = view["baseline_dir"]
        baseline = Image.open(baseline_dir / "baseline.png")
        suppressed = Image.open(baseline_dir / "no-cell-routes.png")
        rendered = Image.open(shot)
        scored = verify.wider(baseline, suppressed, rendered)
        skill_mae = (scored["skill_numerator"] / (scored["skill_pixels"] * 765)
                     if scored["skill_pixels"] else None)
        b = baseline.convert("RGB")
        s = suppressed.convert("RGB")
        r = rendered.convert("RGB")
        bp, sp, rp = b.load(), s.load(), r.load()
        per_box = []
        for i in range(10):
            y0, y1 = 12 + 40 * i, 12 + 40 * i + 32
            numerator = pixels = 0
            for y in range(y0, min(y1, b.height)):
                for x in range(18, min(90, b.width)):
                    if bp[x, y] == sp[x, y]:
                        continue
                    pixels += 1
                    numerator += sum(abs(a - z) for a, z in zip(bp[x, y], rp[x, y]))
            per_box.append({
                "box": i, "numerator": numerator, "pixels": pixels,
                "denominator": pixels * 765,
                "mae": numerator / (pixels * 765) if pixels else None,
            })
        row.update({
            "skill_numerator": scored["skill_numerator"],
            "skill_pixels": scored["skill_pixels"],
            "skill_denominator": scored["skill_pixels"] * 765,
            "skill_mae": skill_mae,
            "skill_pass_le_0_02": (skill_mae is not None and skill_mae <= 0.02),
            "per_box": per_box,
            "wider_numerator": scored["numerator"],
            "wider_denominator": scored["denominator"],
            "wider_mae": scored["mae"],
        })
        results[view["name"]] = row

    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "score-table.json").write_text(json.dumps(results, indent=2), encoding="utf-8", newline="\n")
    print(json.dumps(results, indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
