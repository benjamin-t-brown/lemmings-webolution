import unittest

from PIL import Image, ImageDraw

from match_b1_menu_glyphs import components, ordered_assignment, pixel_error, place_scaled


class GeometryTests(unittest.TestCase):
    def setUp(self):
        self.source = Image.new("L", (3, 4))
        ImageDraw.Draw(self.source).rectangle((0, 0, 1, 3), fill=255)
        self.canvas = place_scaled(self.source, 1.0, 5, 6, (20, 16))

    def test_wrong_scale_changes_error_and_dimensions(self):
        correct = place_scaled(self.source, 1.0, 5, 6, (20, 16))
        wrong = place_scaled(self.source, 2.0, 5, 6, (20, 16))
        self.assertEqual(correct.size, wrong.size)
        self.assertGreater(pixel_error(correct, wrong), 0)

    def test_real_placement_perturbation_worsens_error(self):
        shifted = place_scaled(self.source, 1.0, 6, 6, (20, 16))
        self.assertGreater(pixel_error(self.canvas, shifted), 0)

    def test_component_support_bounds_are_content_derived(self):
        im = Image.new("L", (9, 7))
        ImageDraw.Draw(im).point((2, 3), fill=255)
        ImageDraw.Draw(im).point((7, 1), fill=255)
        self.assertEqual(components(im), [(7, 1, 8, 2), (2, 3, 3, 4)])

    def test_occurrence_reuse_and_reordered_assignment_rejected(self):
        self.assertTrue(ordered_assignment([0, 1, 2, 3]))
        self.assertFalse(ordered_assignment([0, 1, 1, 3]))
        self.assertFalse(ordered_assignment([0, 2, 1, 3]))


if __name__ == "__main__":
    unittest.main()
