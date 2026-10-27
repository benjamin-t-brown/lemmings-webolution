import unittest
from pathlib import Path

from PIL import Image

from tools.harness.menu_image_qualification import (
    NORMAL, ORANGE, qualify_menu_highlight, return_admission,
)

ROOT = Path(__file__).resolve().parents[2]
REFERENCE = ROOT / "build/tmp/b1b-reference-screengrab/menu_highlight0.png"
UNRELATED = ROOT / "build/tmp/b1-facilitator-regression/before-A7.png"


def synthetic(highlights=()):
    im = Image.open(REFERENCE).convert("RGB")
    pixels = im.load()
    for row in range(5):
        y0 = 322 + row * 29
        desired = ORANGE if row in highlights else NORMAL
        for y in range(y0, y0 + 26):
            for x in range(160, 480):
                if pixels[x, y] in (ORANGE, NORMAL):
                    pixels[x, y] = desired
    return im


class MenuImageQualificationTests(unittest.TestCase):
    def test_retained_original_highlight_zero(self):
        with Image.open(REFERENCE) as im:
            q = qualify_menu_highlight(im)
        self.assertEqual(q.highlight, 0, q)

    def test_all_five_declared_states(self):
        for row in range(5):
            with self.subTest(row=row):
                self.assertEqual(qualify_menu_highlight(synthetic((row,))).highlight, row)

    def test_explicit_negative_suite(self):
        cases = {
            "two-highlights": synthetic((0, 1)),
            "no-highlight": synthetic(()),
            "black": Image.new("RGB", (640, 480), "black"),
            "wrong-dimensions": Image.new("RGB", (639, 480), "black"),
            "shifted-cropped": synthetic((0,)).crop((1, 0, 640, 480)),
        }
        for name, image in cases.items():
            with self.subTest(name=name):
                self.assertIsNone(qualify_menu_highlight(image).highlight)
        # The qualifier must identify rows 1..4 (not call them unknown), while
        # the actual RETURN predicate must refuse each one.
        for row in range(1, 5):
            with self.subTest(nonzero_row=row):
                image = synthetic((row,))
                self.assertEqual(qualify_menu_highlight(image).highlight, row)
                self.assertFalse(return_admission(image, image.copy(), menu_identity=True,
                                                  focused=True).allowed)

    def test_same_size_translations_rejected_by_alignment_guard(self):
        original = Image.open(REFERENCE).convert("RGB")
        for dx, dy in ((1, 0), (0, 1), (2, 2), (10, 0), (0, 10),
                       (-1, 0), (0, -1), (-3, 2), (2, -3)):
            with self.subTest(offset=(dx, dy)):
                shifted = Image.new("RGB", original.size, "black")
                shifted.paste(original, (dx, dy))
                q = qualify_menu_highlight(shifted)
                self.assertEqual(q.reason, "frame-misaligned", q)
                self.assertFalse(return_admission(shifted, shifted.copy(),
                                                  menu_identity=True,
                                                  focused=True).allowed)

    def test_unrelated_existing_stock_image_refused(self):
        with Image.open(UNRELATED) as im:
            q = qualify_menu_highlight(im)
        self.assertIsNone(q.highlight, q)

    def test_actual_return_predicate(self):
        good = synthetic((0,))
        self.assertTrue(return_admission(good, good.copy(), menu_identity=True,
                                         focused=True).allowed)
        self.assertFalse(return_admission(good, synthetic((1,)), menu_identity=True,
                                          focused=True).allowed)
        self.assertFalse(return_admission(good, good.copy(), menu_identity=False,
                                          focused=True).allowed)
        self.assertFalse(return_admission(good, good.copy(), menu_identity=True,
                                          focused=False).allowed)
        altered = good.copy(); altered.putpixel((0, 0), (1, 2, 3))
        self.assertFalse(return_admission(good, altered, menu_identity=True,
                                          focused=True).allowed)


if __name__ == "__main__":
    unittest.main()
