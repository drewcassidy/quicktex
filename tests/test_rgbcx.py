import unittest
import os
import rgbcx
import s3tc
from PIL import Image

dir_path = os.path.dirname(os.path.realpath(__file__))
image_path = dir_path + "/images"

# A block that should always encode greyscale, where every row of pixels is identical, and the left side is lighter than the right side
greyscale = Image.open(image_path + "/blocks/greyscale.png").tobytes("raw", "RGBX")

# A block that should always encode 3-color when available.
# from left to right: red, yellow, yellow, green
three_color = Image.open(image_path + "/blocks/3color.png").tobytes("raw", "RGBX")

# A block that should always encode 3-color with black when available
# from left to right: black, red, yellow, green
three_color_black = Image.open(image_path + "/blocks/3color black.png").tobytes("raw", "RGBX")


class TestBC1Encoder(unittest.TestCase):
    """Test BC1 encoder with a variety of inputs with 3 color blocks disabled."""

    def setUp(self):
        self.bc1_encoder = rgbcx.BC1Encoder(rgbcx.InterpolatorType.Ideal, 5, False, False)

    def test_block_size(self):
        """Ensure encoded block size is 8 bytes."""
        out = self.bc1_encoder.encode_image(greyscale, 4, 4)

        self.assertEqual(self.bc1_encoder.block_width, 4, 'incorrect reported block width')
        self.assertEqual(self.bc1_encoder.block_height, 4, 'incorrect reported block height')
        self.assertEqual(self.bc1_encoder.block_size, 8, 'incorrect reported block size')
        self.assertEqual(len(out), 8, 'incorrect returned block size')

    def test_block_4color(self):
        """Ensure encoding the greyscale test block results in a 4 color block."""
        out = s3tc.BC1Block.frombytes(self.bc1_encoder.encode_image(greyscale, 4, 4))

        self.assertFalse(out.is_3color(), "returned 3 color block for greyscale test block")

    def test_block_greyscale_selectors(self):
        """Ensure encoding the greyscale test block results in the correct selectors."""
        out = s3tc.BC1Block.frombytes(self.bc1_encoder.encode_image(greyscale, 4, 4))
        first_row = out.selectors[0]
        expected_row = [0, 2, 3, 1]

        self.assertTrue(all(row == first_row for row in out.selectors), "block has different selectors in each row")
        self.assertTrue(all(row == expected_row for row in out.selectors), "block has incorrect selectors for greyscale test block")

    def test_block_3color(self):
        """Ensure encoder doesn't output a 3 color block."""
        out1 = s3tc.BC1Block.frombytes(self.bc1_encoder.encode_image(three_color, 4, 4))
        out2 = s3tc.BC1Block.frombytes(self.bc1_encoder.encode_image(three_color_black, 4, 4))

        self.assertFalse(out1.is_3color(), "returned 3 color block with use_3color disabled")
        self.assertFalse(out2.is_3color(), "returned 3 color block with use_3color disabled")


class TestBC1Encoder3Color(unittest.TestCase):
    """Test BC1 encoder with a variety of inputs with 3 color blocks enabled."""

    def setUp(self):
        self.bc1_encoder = rgbcx.BC1Encoder(rgbcx.InterpolatorType.Ideal, 5, True, False)

    def test_block_4color(self):
        """Ensure encoding the greyscale test block results in a 4 color block."""
        out = s3tc.BC1Block.frombytes(self.bc1_encoder.encode_image(greyscale, 4, 4))

        self.assertFalse(out.is_3color(), "returned 3 color block for greyscale test block")

    def test_block_3color(self):
        """Ensure encoding the 3 color test block results in a 3 color block."""
        out = s3tc.BC1Block.frombytes(self.bc1_encoder.encode_image(three_color, 4, 4))

        self.assertTrue(out.is_3color(), "returned 4 color block with use_3color enabled")

    def test_block_3color_black(self):
        """Ensure encoder doesn't output a 3 color block with black pixels."""
        out = s3tc.BC1Block.frombytes(self.bc1_encoder.encode_image(three_color_black, 4, 4))

        self.assertFalse(out.is_3color() and any(3 in row for row in out.selectors),
                         "returned 3 color block with black pixels with use_3color_black disabled")

    def test_block_selectors(self):
        """Ensure encoding the 3 color test block results in the correct selectors."""
        out = s3tc.BC1Block.frombytes(self.bc1_encoder.encode_image(three_color, 4, 4))
        first_row = out.selectors[0]
        expected_row = [1, 2, 2, 0]

        self.assertTrue(all(row == first_row for row in out.selectors), "block has different selectors in each row")
        self.assertTrue(all(row == expected_row for row in out.selectors), "block has incorrect selectors for 3 color test block")


class TestBC1Encoder3ColorBlack(unittest.TestCase):
    """ Test BC1 encoder with a variety of inputs with 3 color blocks with black pixels enabled"""

    def setUp(self):
        self.bc1_encoder = rgbcx.BC1Encoder(rgbcx.InterpolatorType.Ideal, 5, True, True)

    def test_block_3color(self):
        out = s3tc.BC1Block.frombytes(self.bc1_encoder.encode_image(three_color, 4, 4))

        self.assertTrue(out.is_3color(), "returned 4 color block with use_3color enabled")

    def test_block_3color_black(self):
        out = s3tc.BC1Block.frombytes(self.bc1_encoder.encode_image(three_color_black, 4, 4))

        self.assertTrue(out.is_3color(), "returned 4 color block with use_3color enabled")
        self.assertTrue(any(3 in row for row in out.selectors), "returned block without black pixels with use_3color_black enabled")

    def test_block_selectors(self):
        out = s3tc.BC1Block.frombytes(self.bc1_encoder.encode_image(three_color_black, 4, 4))
        first_row = out.selectors[0]
        expected_row = [3, 1, 2, 0]

        self.assertTrue(all(row == first_row for row in out.selectors), "block has different selectors in each row")
        self.assertTrue(all(row == expected_row for row in out.selectors), "block has incorrect selectors for 3 color black test block")


if __name__ == '__main__':
    unittest.main()
