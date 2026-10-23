import unittest

from PIL import Image

from score_b1_menu import score


class ScoreControls(unittest.TestCase):
    def test_identical_frames_score_zero(self):
        a = Image.new("RGB", (640, 480), (10, 20, 30))
        r, _ = score(a, a.copy())
        self.assertEqual((r["numerator"], r["changed_pixels"], r["mae"]), (0, 0, 0.0))
        self.assertTrue(r["passes"])

    def test_one_pixel_counts_exactly(self):
        a = Image.new("RGB", (640, 480), (0, 0, 0))
        b = a.copy()
        b.putpixel((5, 7), (255, 0, 3))
        r, _ = score(a, b)
        self.assertEqual((r["numerator"], r["changed_pixels"]), (258, 1))
        self.assertAlmostEqual(r["mae"], 258 / (255 * 3 * 640 * 480))

    def test_gate_fails_above_threshold(self):
        a = Image.new("RGB", (640, 480), (0, 0, 0))
        b = Image.new("RGB", (640, 480), (7, 7, 7))
        r, _ = score(a, b)  # 7/255 = 0.0275
        self.assertFalse(r["passes"])


if __name__ == "__main__":
    unittest.main()
