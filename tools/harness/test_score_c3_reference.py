"""Regression tests for C3's explicitly selected scoring reference."""

import hashlib
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[2]
SCORER_PATH = ROOT / "tools/harness/score_c3_regions.py"
def write_png(path: Path, color: tuple[int, int, int], pixel=None) -> None:
    image = Image.new("RGB", (640, 480), color)
    if pixel is not None:
        image.putpixel(pixel[0], pixel[1])
    image.save(path)


class ExplicitReferenceTests(unittest.TestCase):
    def test_cli_scores_selected_synthetic_reference_and_reports_identity(self):
        with tempfile.TemporaryDirectory() as td:
            tmp = Path(td)
            ref0, ref255, candidate, output = (tmp / n for n in
                                                ("ref0.png", "ref255.png", "candidate.png", "score.json"))
            write_png(ref0, (0, 0, 0))
            write_png(ref255, (0, 0, 0), ((500, 200), (255, 0, 0)))
            write_png(candidate, (0, 0, 0), ((500, 200), (0, 0, 0)))

            observed = []
            for reference in (ref0, ref255):
                # Exercise the actual command entry point in a subprocess, including --reference.
                result = subprocess.run(
                    [sys.executable, str(SCORER_PATH), "--reference", str(reference),
                     "--candidate", str(candidate), "--control", str(tmp / "absent.png"),
                     "--json", str(output)],
                    cwd=ROOT, capture_output=True, text=True, check=False)
                self.assertEqual(result.returncode, 0, result.stderr)
                report = json.loads(output.read_text(encoding="utf-8"))
                self.assertEqual(report["reference"], str(reference.resolve()))
                self.assertEqual(report["reference_sha256"], hashlib.sha256(reference.read_bytes()).hexdigest())
                self.assertIn(str(reference.resolve()), result.stdout)
                self.assertIn(report["reference_sha256"], result.stdout)
                run = report["runs"]["candidate (hud 70 ticks + caption)"]
                self.assertTrue(run["partition_exact"])
                self.assertEqual(run["sum_numerators"], run["full_frame_numerator"])
                self.assertEqual(run["sum_pixels"], 640 * 480)
                observed.append(run["full_frame_numerator"])
            self.assertEqual(observed, [0, 255])

    def test_repair4_and_historical_a6d3_real_reference_reruns(self):
        repair4 = ROOT / "build/tmp/a6f-fresh-pause-gate/repair4/original-live.png"
        legacy = ROOT / "build/tmp/a6d3-matched-state-bbox/original-live.png"
        candidate = ROOT / "build/tmp/c3a-residual-breakdown/render-hud70-caption.png"
        if not all(p.is_file() for p in (repair4, legacy, candidate)):
            self.skipTest("recorded C3 captures/render are not present in this checkout")
        with tempfile.TemporaryDirectory() as td:
            out = Path(td) / "score.json"
            reports = []
            for ref in (repair4, legacy):
                result = subprocess.run(
                    [sys.executable, str(SCORER_PATH), "--reference", str(ref),
                     "--candidate", str(candidate), "--control", str(Path(td) / "none.png"),
                     "--json", str(out)], cwd=ROOT, capture_output=True, text=True, check=False)
                self.assertEqual(result.returncode, 0, result.stderr)
                report = json.loads(out.read_text(encoding="utf-8"))
                run = report["runs"]["candidate (hud 70 ticks + caption)"]
                self.assertEqual(report["reference_sha256"], hashlib.sha256(ref.read_bytes()).hexdigest())
                self.assertTrue(run["partition_exact"])
                self.assertEqual(run["sum_numerators"], run["full_frame_numerator"])
                self.assertEqual(run["sum_pixels"], 640 * 480)
                reports.append((report, run))
            self.assertNotEqual(reports[0][0]["reference_sha256"], reports[1][0]["reference_sha256"])
            self.assertNotEqual(reports[0][1]["full_frame_numerator"], reports[1][1]["full_frame_numerator"])
            self.assertEqual(reports[0][1]["full_frame_numerator"], 49_336_286)
            self.assertEqual(reports[1][1]["full_frame_numerator"], 49_297_573)
            self.assertEqual([r["numerator"] for r in reports[0][1]["rows"]],
                             [5_552_195, 822_566, 412_464, 61_588, 708_159, 41_779_314])
            self.assertEqual([r["numerator"] for r in reports[1][1]["rows"]],
                             [5_546_337, 836_236, 412_464, 44_746, 708_159, 41_749_631])
            expected_pixels = [23_040, 2_436, 2_752, 630, 59_400, 218_942]
            self.assertEqual([r["pixels"] for r in reports[0][1]["rows"]], expected_pixels)
            self.assertEqual([r["pixels"] for r in reports[1][1]["rows"]], expected_pixels)

            # No --reference must still reproduce the old A6d3 default exactly, but label it.
            result = subprocess.run(
                [sys.executable, str(SCORER_PATH), "--candidate", str(candidate),
                 "--control", str(Path(td) / "none.png"), "--json", str(out)],
                cwd=ROOT, capture_output=True, text=True, check=False)
            self.assertEqual(result.returncode, 0, result.stderr)
            default_report = json.loads(out.read_text(encoding="utf-8"))
            default_run = default_report["runs"]["candidate (hud 70 ticks + caption)"]
            self.assertEqual(default_report["reference_kind"], "historical A6d3 default")
            self.assertEqual(default_report["reference_sha256"], reports[1][0]["reference_sha256"])
            self.assertEqual(default_run["full_frame_numerator"], reports[1][1]["full_frame_numerator"])
            self.assertIn("historical A6d3 default", result.stdout)

    def test_missing_candidate_is_not_reported_as_success(self):
        result = subprocess.run(
            [sys.executable, str(SCORER_PATH), "--candidate", str(ROOT / "no-such-candidate.png")],
            cwd=ROOT, capture_output=True, text=True, check=False)
        self.assertEqual(result.returncode, 2)
        self.assertIn("candidate missing", result.stderr)


if __name__ == "__main__":
    unittest.main()
