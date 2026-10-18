"""Independent camera screenshot check under the preregistered A1 policy."""
import argparse
import hashlib
import json
from pathlib import Path
from PIL import Image, ImageChops

p = argparse.ArgumentParser()
p.add_argument("reference", type=Path)
p.add_argument("candidate", type=Path)
p.add_argument("--roi", type=int, nargs=4, required=True, metavar=("X0", "Y0", "X1", "Y1"))
p.add_argument("--baseline-error", type=float)
p.add_argument("--diff", type=Path)
a = p.parse_args()
reference = Image.open(a.reference).convert("RGB")
candidate = Image.open(a.candidate).convert("RGB")
assert reference.size == candidate.size == (640, 480), "No implicit resizing or registration permitted"
x0, y0, x1, y1 = a.roi
assert 0 <= x0 < x1 <= 640 and 0 <= y0 < y1 <= 480
diff = ImageChops.difference(reference.crop(a.roi), candidate.crop(a.roi))
hist = diff.histogram()
total = sum((i % 256) * count for i, count in enumerate(hist))
error = total / (255 * 3 * (x1-x0) * (y1-y0))
report = {
    "reference_sha256": hashlib.sha256(a.reference.read_bytes()).hexdigest(),
    "candidate_sha256": hashlib.sha256(a.candidate.read_bytes()).hexdigest(),
    "roi": a.roi,
    "absolute_rgb_difference_sum": total,
    "mean_absolute_rgb_error": error,
    "threshold": 0.02,
    "passes_threshold": error <= 0.02,
    "baseline_error": a.baseline_error,
    "mutation_worsens_error": None if a.baseline_error is None else error > a.baseline_error,
}
if a.diff:
    diff.save(a.diff)
print(json.dumps(report, indent=2))
if a.baseline_error is not None:
    raise SystemExit(0 if error > a.baseline_error else 1)
raise SystemExit(0 if error <= 0.02 else 1)
