import unittest

from PIL import Image, ImageDraw

from match_b1_menu_glyphs import (components, composite_candidate, render_row, rgb_error,
                                  target_mask, place_scaled, pixel_error,
                                  ordered_assignment)


class RegistrationTests(unittest.TestCase):
    def setUp(self):
        self.source = Image.new("L", (3, 4))
        ImageDraw.Draw(self.source).rectangle((0, 0, 1, 3), fill=255)
        self.canvas = place_scaled(self.source, 1.0, 5, 6, (20, 16))

    def test_component_support_bounds_are_content_derived(self):
        im = Image.new("L", (9, 7))
        ImageDraw.Draw(im).point((2, 3), fill=255)
        ImageDraw.Draw(im).point((7, 1), fill=255)
        self.assertEqual(components(im), [(7, 1, 8, 2), (2, 3, 3, 4)])

    def test_wrong_scale_changes_common_canvas_error(self):
        correct = place_scaled(self.source, 1.0, 5, 6, (20, 16))
        wrong = place_scaled(self.source, 2.0, 5, 6, (20, 16))
        self.assertEqual(correct.size, wrong.size)
        self.assertGreater(pixel_error(correct, wrong), 0)

    def test_real_placement_perturbation_worsens_error(self):
        exact = place_scaled(self.source, 1.0, 5, 6, (20, 16))
        shifted = place_scaled(self.source, 1.0, 6, 6, (20, 16))
        self.assertGreater(pixel_error(exact, shifted), 0)

    def test_ordered_assignment_rejects_reuse_or_reordering(self):
        self.assertTrue(ordered_assignment([0, 1, 2, 3]))
        self.assertFalse(ordered_assignment([0, 1, 1, 3]))
        self.assertFalse(ordered_assignment([0, 2, 1, 3]))

    def test_actual_registration_scale_and_placement_controls(self):
        sheet = Image.new("RGBA", (8, 6), (0, 0, 0, 0))
        # Real source crop with two separated bars: geometry, alpha and compositing matter.
        ImageDraw.Draw(sheet).rectangle((1, 1, 2, 4), fill=(220, 20, 5, 255))
        rect = (1, 1, 3, 5)
        target = Image.new("RGB", (20, 10), (224, 10, 0))
        patch = Image.new("RGBA", (2, 4), (224, 10, 0, 255))
        glyph = composite_candidate(sheet, rect, 1.0)
        black_ink = Image.new("RGBA", glyph.size, (0, 0, 0, 0))
        black_ink.putalpha(glyph.getchannel("A"))
        patch.alpha_composite(black_ink)
        target.paste(patch.convert("RGB"), (6, 3))
        placements = [{"source_rect": list(rect), "destination_rect": [6, 3, 8, 7]}]
        exact = render_row(sheet, placements, 1.0, target.size, (224, 10, 0))
        wrong_scale = render_row(sheet, placements, 1.5, target.size, (224, 10, 0))
        shifted = [{"source_rect": list(rect), "destination_rect": [7, 3, 9, 7]}]
        wrong_place = render_row(sheet, shifted, 1.0, target.size, (224, 10, 0))
        self.assertEqual(rgb_error(target, exact), 0)
        self.assertGreater(rgb_error(target, wrong_scale), 0)
        self.assertGreater(rgb_error(target, wrong_place), 0)

    def test_ordered_row_can_score_merged_support_without_four_clusters(self):
        sheet = Image.new("RGBA", (16, 8), (0, 0, 0, 0))
        draw = ImageDraw.Draw(sheet)
        # Four independent retail-shaped glyph supports (vertical strokes and crossbar).
        rects = [(1, 1, 4, 7), (5, 1, 8, 7), (9, 1, 12, 7), (13, 1, 16, 7)]
        for x0, y0, x1, y1 in rects:
            draw.rectangle((x0, y0, x1 - 1, y1 - 1), fill=(240, 30, 0, 255))
        # Match the renderer's black-ink tint while retaining source alpha.
        target = Image.new("RGB", (19, 8), (224, 10, 0))
        for x in (1, 5, 9, 13):
            target.paste((0, 0, 0), (x, 1, x + 3, 7))
        # Touch target supports into one connected component while retaining four source
        # placements and ordered physical positions on a common raster.
        for x in (4, 8, 12):
            target.putpixel((x, 3), (0, 0, 0))
        mask = target_mask(target)
        self.assertEqual(len(components(mask)), 1)
        placements = [{"source_rect": list(rect), "destination_rect": [x, 1, x + 3, 7]}
                      for rect, x in zip(rects, (1, 5, 9, 13))]
        rendered = render_row(sheet, placements, 1.0, target.size, (224, 10, 0))
        self.assertLess(rgb_error(target, rendered), rgb_error(target, Image.new("RGB", target.size, (224, 10, 0))))
        # Four alpha-derived support rectangles are registered without codepoint indexing.
        self.assertEqual(components(sheet.getchannel("A")), rects)


if __name__ == "__main__":
    unittest.main()
