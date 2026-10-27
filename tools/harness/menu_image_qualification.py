"""Fail-closed image-only qualifier for the fixed 640x480 B2 menu.

No capture, process, input, or native-memory operations are performed here.
"""
from __future__ import annotations

from dataclasses import dataclass
from hashlib import sha256
from pathlib import Path
import sys
from typing import Any

from PIL import Image

SIZE = (640, 480)
ORANGE = (224, 128, 0)
NORMAL = (224, 10, 0)
MIN_PIXELS = 100  # frozen absolute floor; no tolerance or image-derived fitting
X0, X1, X_STEP = 160, 480, 2
Y0, ROW_STEP, BAND_HEIGHT = 322, 29, 26
ALIGNMENT_ENVELOPE = (160, 320, 480, 468)  # excludes the menu panels/glyph rows
REFERENCE_PATH = Path(__file__).resolve().parents[2] / "build/tmp/b1b-reference-screengrab/menu_highlight0.png"


@dataclass(frozen=True)
class RowMetrics:
    orange: int
    normal: int


@dataclass(frozen=True)
class Qualification:
    highlight: int | None
    reason: str
    row_metrics: tuple[RowMetrics, ...]


@dataclass(frozen=True)
class Admission:
    allowed: bool
    reason: str
    qualification: Qualification


def _rgb_image(image: Any) -> Image.Image | None:
    if not isinstance(image, Image.Image) or image.size != SIZE:
        return None
    try:
        return image.convert("RGB")
    except (ValueError, OSError):
        return None


def _reference_aligned(rgb: Image.Image) -> bool:
    """Require exact retained-reference pixels outside the declared menu envelope."""
    try:
        with Image.open(REFERENCE_PATH) as source:
            reference = source.convert("RGB")
    except (OSError, ValueError):
        return False
    if reference.size != SIZE:
        return False
    x0, y0, x1, y1 = ALIGNMENT_ENVELOPE
    actual, expected = rgb.load(), reference.load()
    for y in range(SIZE[1]):
        for x in range(SIZE[0]):
            if x0 <= x < x1 and y0 <= y < y1:
                continue
            if actual[x, y] != expected[x, y]:
                return False
    return True


def qualify_menu_highlight(image: Any) -> Qualification:
    """Identify exactly one highlighted row, else return unknown with diagnostics."""
    rgb = _rgb_image(image)
    if rgb is None:
        return Qualification(None, "wrong-image-type-or-dimensions", ())
    if not _reference_aligned(rgb):
        return Qualification(None, "frame-misaligned", ())
    pix = rgb.load()
    rows: list[RowMetrics] = []
    for row in range(5):
        orange = normal = 0
        ya = Y0 + row * ROW_STEP
        for y in range(ya, ya + BAND_HEIGHT):
            for x in range(X0, X1, X_STEP):
                color = pix[x, y]
                orange += color == ORANGE
                normal += color == NORMAL
        rows.append(RowMetrics(orange, normal))
    hits = [i for i, m in enumerate(rows) if m.orange >= MIN_PIXELS]
    if len(hits) != 1:
        return Qualification(None, "no-highlight" if not hits else "multiple-highlights", tuple(rows))
    hit = hits[0]
    if any(rows[i].normal < MIN_PIXELS for i in range(5) if i != hit):
        return Qualification(None, "unsupported-normal-row", tuple(rows))
    return Qualification(hit, "qualified", tuple(rows))


def return_admission(first: Any, second: Any, *, menu_identity: bool,
                     focused: bool) -> Admission:
    """Gate RETURN on two identical fresh images plus caller-proven identity/focus.

    Caller must acquire both frames freshly from the same observation. This API
    cannot establish freshness; callers attest via this documented contract.
    """
    q = qualify_menu_highlight(first)
    if not menu_identity:
        return Admission(False, "menu-identity-unverified", q)
    if not focused:
        return Admission(False, "not-focused", q)
    a, b = _rgb_image(first), _rgb_image(second)
    if a is None or b is None:
        return Admission(False, "invalid-frame", q)
    if a.tobytes() != b.tobytes():
        return Admission(False, "unstable-frames", q)
    if q.highlight != 0:
        return Admission(False, "highlight-not-exactly-zero", q)
    return Admission(True, "admitted", q)


def _main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {Path(sys.argv[0]).name} IMAGE.png", file=sys.stderr)
        return 2
    path = Path(sys.argv[1])
    image = Image.open(path)
    result = qualify_menu_highlight(image)
    print({"path": str(path), "sha256": sha256(path.read_bytes()).hexdigest(),
           "highlight": result.highlight, "reason": result.reason,
           "rows": [m.__dict__ for m in result.row_metrics]})
    return 0 if result.highlight is not None else 1


if __name__ == "__main__":
    raise SystemExit(_main())
