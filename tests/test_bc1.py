import unittest
import nose
from parameterized import parameterized_class
from s3tc import BC1Block
from images import Blocks
# import quicktex.s3tc.bc1 as bc1
# import quicktex.s3tc.interpolator as interpolator
#
# ColorMode = bc1.BC1Encoder.ColorMode

#
# class TestBC1Encoder(unittest.TestCase):
#     """Test BC1 Encoder"""
#
#     def setUp(self):
#         self.bc1_encoder = bc1.BC1Encoder(5)
#
#     def test_block_size(self):
#         """Ensure encoded block size is 8 bytes."""
#         out = self.bc1_encoder.encode_image(Blocks.greyscale, 4, 4)
#
#         self.assertEqual(self.bc1_encoder.block_width, 4, 'incorrect reported block width')
#         self.assertEqual(self.bc1_encoder.block_height, 4, 'incorrect reported block height')
#         self.assertEqual(self.bc1_encoder.block_size, 8, 'incorrect reported block size')
#         self.assertEqual(len(out), 8, 'incorrect returned block size')
#
#
# def get_class_name_blocks(cls, num, params_dict):
#     return "%s%s" % (cls.__name__, params_dict['color_mode'].name,)
#
#
# @parameterized_class([
#     {"color_mode": ColorMode.FourColor},
#     {"color_mode": ColorMode.ThreeColor},
#     {"color_mode": ColorMode.ThreeColorBlack},
# ], class_name_func=get_class_name_blocks)
# class TestBC1EncoderBlocks(unittest.TestCase):
#     """Test BC1 encoder with a variety of inputs with 3 color blocks disabled."""
#
#     def setUp(self):
#         self.bc1_encoder = bc1.BC1Encoder(5, self.color_mode)
#
#     def test_block_4color(self):
#         """Test encoder output with 4 color greyscale testblock."""
#         out = BC1Block.frombytes(self.bc1_encoder.encode_image(Blocks.greyscale, 4, 4))
#         selectors = [[0, 2, 3, 1]] * 4
#
#         self.assertFalse(out.is_3color(), "returned block color mode for greyscale test block")
#         self.assertEqual(selectors, out.selectors, "block has incorrect selectors for greyscale test block")
#
#     def test_block_3color(self):
#         """Test encoder output with 3 color test block."""
#         out = BC1Block.frombytes(self.bc1_encoder.encode_image(Blocks.three_color, 4, 4))
#         selectors = [[1, 2, 2, 0]] * 4
#
#         if self.color_mode != ColorMode.FourColor:  # we only care about the selectors if we are in 3 color mode
#             self.assertTrue(out.is_3color(), "returned 4-color block for 3 color test block")
#             self.assertEqual(selectors, out.selectors, "block has incorrect selectors for 3 color test block")
#         else:
#             self.assertFalse(out.is_3color(), "return 3-color block in 4-color mode")
#
#     def test_block_3color_black(self):
#         """Test encoder output with 3 color test block with black pixels."""
#         out = BC1Block.frombytes(self.bc1_encoder.encode_image(Blocks.three_color_black, 4, 4))
#         selectors = [[3, 1, 2, 0]] * 4
#
#         if self.color_mode == ColorMode.ThreeColorBlack:  # we only care about the selectors if we are in 3 color black mode
#             self.assertTrue(out.is_3color_black(), "returned 4-color block for 3 color test block with black")
#             self.assertEqual(selectors, out.selectors, "block has incorrect selectors for 3 color with black test block")
#         else:
#             self.assertFalse(out.is_3color_black(), "returned incorrect block color mode for 3 color with black test block")
#
#
# if __name__ == '__main__':
#     nose.main()
