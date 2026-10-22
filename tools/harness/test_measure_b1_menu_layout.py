import unittest
from PIL import Image

from measure_b1_menu_layout import locate, panel_runs, alpha_components, glyph_search


class MatchControls(unittest.TestCase):
    def test_glyph_alpha_path_matches_shape_without_rgb_identity(self):
        sheet=Image.new('RGBA',(256,240),(0,0,0,0))
        # ASCII 'A' cell 33; source alpha silhouette is deliberately white.
        for x,y in ((1,1),(2,1),(0,2),(1,2),(2,2)):
            sheet.putpixel((33%16*16+x,33//16*16+y),(255,255,255,255))
        row=Image.new('RGB',(32,20),(224,10,0))
        for x,y in ((4,5),(5,5),(6,5),(3,6),(4,6),(5,6)):
            row.putpixel((x,y),(0,0,0))
        result=glyph_search(sheet,row,'A',(0,0,32,20),scales=(1,))
        self.assertEqual(result['matches'][0]['char'],'A')
        self.assertLess(result['matches'][0]['score'],.2)
        self.assertEqual(result['matches'][0]['source'][0],33%16*16)

    def test_panel_detection_bridges_dark_glyph_holes_and_narrow_bar(self):
        im = Image.new("RGB", (120, 80), (0, 0, 0))
        from PIL import ImageDraw
        d = ImageDraw.Draw(im)
        d.rectangle((10, 42, 99, 55), fill=(220, 20, 0))
        d.rectangle((20, 47, 79, 49), fill=(0, 0, 0))
        d.rectangle((30, 60, 89, 63), fill=(220, 20, 0))
        self.assertEqual(panel_runs(im), [dict(x0=10,y0=42,x1=100,y1=56),
                                        dict(x0=30,y0=60,x1=90,y1=64)])

    def test_atlas_alpha_support_returns_source_rectangles(self):
        im=Image.new("RGBA",(12,8),(255,255,255,0))
        for x,y in ((2,2),(2,3),(3,3),(8,4),(9,4),(9,5)):
            im.putpixel((x,y),(255,255,255,255))
        self.assertEqual(alpha_components(im),[(2,2,4,4,3),(8,4,10,6,3)])
    def test_unique_exact_template(self):
        ref = Image.new("RGB", (8, 8), "black")
        t = Image.new("RGB", (2, 2), "white")
        ref.paste(t, (3, 4))
        fits = locate(ref, t)
        self.assertEqual(fits[0][1:4], (3, 4, 1))
        self.assertEqual(fits[0][0], 0)

    def test_ambiguous_repeated_template_is_reported_by_equal_top_scores(self):
        ref = Image.new("RGB", (8, 8), "black")
        t = Image.new("RGB", (2, 2), "white")
        ref.paste(t, (1, 1)); ref.paste(t, (5, 5))
        fits = locate(ref, t)
        self.assertEqual(fits[0][0], fits[1][0])

    def test_one_pixel_perturbation_increases_exact_fit_error(self):
        t = Image.new("RGB", (2, 2), "white")
        good = Image.new("RGB", (8, 8), "black"); good.paste(t, (3, 3))
        bad = good.copy(); bad.putpixel((3, 3), (0, 0, 0))
        self.assertLess(locate(good, t)[0][0], locate(bad, t)[0][0])

    def test_wrong_template_negative(self):
        ref = Image.new("RGB", (8, 8), "black")
        t = Image.new("RGB", (2, 2), "white"); ref.paste(t, (2, 2))
        wrong = Image.new("RGB", (2, 2), "red")
        self.assertLess(locate(ref, t)[0][0], locate(ref, wrong)[0][0])


if __name__ == "__main__":
    unittest.main()
