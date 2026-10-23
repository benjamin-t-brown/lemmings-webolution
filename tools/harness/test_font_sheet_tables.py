import unittest
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from PIL import Image
from check_font_sheet_tables import check_cell

class Predicates(unittest.TestCase):
    def test_wrong_coordinate_is_detected_as_bounds(self):
        im=Image.new('RGBA',(4,4),(0,0,0,0))
        c=check_cell(im,[3,0,2],2,ord('Q'),0x20)
        self.assertFalse(c['bounds'])
        self.assertEqual(c['reason'],'rectangle out of bounds')

    def test_wrong_width_expands_rectangle_and_fails(self):
        im=Image.new('RGBA',(4,3),(0,0,0,0))
        self.assertTrue(check_cell(im,[1,0,2],2,ord('M'),0x20)['bounds'])
        self.assertFalse(check_cell(im,[1,0,4],2,ord('M'),0x20)['bounds'])

    def test_blank_predicate_distinguishes_ink(self):
        blank=Image.new('RGBA',(4,3),(0,0,0,0))
        lit=blank.copy(); lit.putpixel((1,0),(255,255,255,255))
        self.assertIs(check_cell(blank,[1,0,2],2,0x20,0x20)['ink'],False)
        self.assertIs(check_cell(lit,[1,0,2],2,0x20,0x20)['ink'],True)

if __name__=='__main__': unittest.main()
