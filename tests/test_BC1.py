import unittest

import nose
import parameterized
import s3tc
import rgbcx
from images import Blocks


def get_class_name_bc1encoder(cls, num, params_dict):
    # By default the generated class named includes either the "name"
    # parameter (if present), or the first string value. This example shows
    # multiple parameters being included in the generated class name:
    return "%s%s" % (
        cls.__name__,
        params_dict['suffix'],
    )


@parameterized.parameterized_class([
    {"use_3color": False, "use_3color_black": False, "suffix": "4Color"},
    {"use_3color": True, "use_3color_black": False, "suffix": "3Color"},
    {"use_3color": True, "use_3color_black": True, "suffix": "3ColorBlack"},
], class_name_func=get_class_name_bc1encoder)
class TestBC1EncoderBlocks(unittest.TestCase):
    """Test BC1 encoder with a variety of inputs with 3 color blocks disabled."""

    def setUp(self):
        self.bc1_encoder = rgbcx.BC1Encoder(rgbcx.InterpolatorType.Ideal, 5, self.use_3color, self.use_3color_black)

    def test_block_size(self):
        """Ensure encoded block size is 8 bytes."""
        out = self.bc1_encoder.encode_image(Blocks.greyscale, 4, 4)

        self.assertEqual(self.bc1_encoder.block_width, 4, 'incorrect reported block width')
        self.assertEqual(self.bc1_encoder.block_height, 4, 'incorrect reported block height')
        self.assertEqual(self.bc1_encoder.block_size, 8, 'incorrect reported block size')
        self.assertEqual(len(out), 8, 'incorrect returned block size')

    def test_block_4color(self):
        """Test encoder output with 4 color greyscale testblock."""
        out = s3tc.BC1Block.frombytes(self.bc1_encoder.encode_image(Blocks.greyscale, 4, 4))
        selectors = [[0, 2, 3, 1]] * 4

        self.assertFalse(out.is_3color(), "returned block color mode for greyscale test block")
        self.assertEqual(selectors, out.selectors, "block has incorrect selectors for greyscale test block")

    def test_block_3color(self):
        """Test encoder output with 3 color test block."""
        out = s3tc.BC1Block.frombytes(self.bc1_encoder.encode_image(Blocks.three_color, 4, 4))
        selectors = [[1, 2, 2, 0]] * 4

        self.assertEqual(out.is_3color(), self.use_3color, "returned incorrect block color mode for 3 color test block")
        if self.use_3color:  # we only care about the selectors if we are in 3 color mode
            self.assertEqual(selectors, out.selectors, "block has incorrect selectors for 3 color test block")

    def test_block_3color_black(self):
        """Test encoder output with 3 color test block with black pixels."""
        out = s3tc.BC1Block.frombytes(self.bc1_encoder.encode_image(Blocks.three_color_black, 4, 4))
        selectors = [[3, 1, 2, 0]] * 4

        self.assertEqual(out.is_3color_black(), self.use_3color_black, "returned incorrect block color mode for 3 color with black test block")
        if self.use_3color_black:  # we only care about the selectors if we are in 3 color black mode
            self.assertEqual(selectors, out.selectors, "block has incorrect selectors for 3 color with black test block")


if __name__ == '__main__':
    nose.main()
