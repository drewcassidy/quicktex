import unittest
import python_rgbcx
import color
import os
import s3tc
from PIL import Image

dir_path = os.path.dirname(os.path.realpath(__file__))
image_path = dir_path + "/images"

# A block that should always encode greyscale, where every row of pixels is identical, and the left side is lighter than the right side
greyscale = Image.open(image_path + "/blocks/greyscale.png").tobytes("raw", "RGBX")

# A block that should always encode 3-color when available
three_color = Image.open(image_path + "/blocks/3color.png").tobytes("raw", "RGBX")

# A block that should always encode 3-color with black when available
three_color_black = Image.open(image_path + "/blocks/3color black.png").tobytes("raw", "RGBX")


class TestBC1Encoder(unittest.TestCase):
    def setUp(self):
        self.bc1_encoder = python_rgbcx.BC1Encoder(python_rgbcx.InterpolatorType.Ideal, 5, False, False)

    def test_block_size(self):
        out = self.bc1_encoder.encode_image(greyscale, 4, 4)

        self.assertEqual(self.bc1_encoder.block_width, 4, 'incorrect reported block width')
        self.assertEqual(self.bc1_encoder.block_height, 4, 'incorrect reported block height')
        self.assertEqual(self.bc1_encoder.block_size, 8, 'incorrect reported block size')
        self.assertEqual(len(out), 8, 'incorrect returned block size')

    def test_block_3color(self):
        out = s3tc.BC1Block.from_bytes(self.bc1_encoder.encode_image(three_color_black, 4, 4))

        self.assertFalse(out.is_3color(), "returned 3 color block with use_3color disabled")


class TestBC1Encoder3Color(unittest.TestCase):
    def setUp(self):
        self.bc1_encoder = python_rgbcx.BC1Encoder(python_rgbcx.InterpolatorType.Ideal, 5, True, False)

    def test_block_3color(self):
        out = s3tc.BC1Block.from_bytes(self.bc1_encoder.encode_image(three_color, 4, 4))

        self.assertTrue(out.is_3color(), "returned 4 color block with use_3color enabled")

    def test_block_3color_black(self):
        out = s3tc.BC1Block.from_bytes(self.bc1_encoder.encode_image(three_color_black, 4, 4))

        self.assertFalse(out.is_3color() and any(3 in row for row in out.selectors),
                         "returned 3 color block with black pixels with use_3color_black disabled")


class TestBC1Encoder3ColorBlack(unittest.TestCase):
    def setUp(self):
        self.bc1_encoder = python_rgbcx.BC1Encoder(python_rgbcx.InterpolatorType.Ideal, 5, True, True)

    def test_block_3color(self):
        out = s3tc.BC1Block.from_bytes(self.bc1_encoder.encode_image(three_color, 4, 4))

        self.assertTrue(out.is_3color(), "returned 4 color block with use_3color enabled")

    def test_block_3color_black(self):
        out = s3tc.BC1Block.from_bytes(self.bc1_encoder.encode_image(three_color_black, 4, 4))

        self.assertTrue(out.is_3color(), "returned 4 color block with use_3color enabled")
        self.assertTrue(any(3 in row for row in out.selectors), "returned block without black pixels with use_3color_black enabled")


if __name__ == '__main__':
    unittest.main()
