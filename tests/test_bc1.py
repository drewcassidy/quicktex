import unittest
import nose
from parameterized import parameterized, parameterized_class
import quicktex.s3tc.bc1 as bc1

in_endpoints = ((253, 254, 255), (65, 70, 67))  # has some small changes that should encode the same
out_endpoints = ((255, 255, 255, 255), (66, 69, 66, 255))
selectors = [[0, 2, 3, 1]] * 4
block_bytes = b'\xff\xff\x28\x42\x78\x78\x78\x78'


class TestBC1Block(unittest.TestCase):
    """Tests for the BC1Block class"""

    def test_size(self):
        """Test the size and dimensions of BC1Block"""
        self.assertEqual(bc1.BC1Block.size, 8, 'incorrect block size')
        self.assertEqual(bc1.BC1Block.width, 4, 'incorrect block width')
        self.assertEqual(bc1.BC1Block.height, 4, 'incorrect block width')
        self.assertEqual(bc1.BC1Block.dimensions, (4, 4), 'incorrect block dimensions')

    def test_buffer(self):
        """Test the buffer protocol of BC1Block"""
        block = bc1.BC1Block()
        mv = memoryview(block)

        self.assertFalse(mv.readonly, 'buffer is readonly')
        self.assertTrue(mv.c_contiguous, 'buffer is not contiguous')
        self.assertEqual(mv.ndim, 1, 'buffer is multidimensional')
        self.assertEqual(mv.nbytes, bc1.BC1Block.size, 'buffer is the wrong size')
        self.assertEqual(mv.format, 'B', 'buffer has the wrong format')

        mv[:] = block_bytes
        self.assertEqual(mv.tobytes(), block_bytes, 'incorrect buffer data')

    def test_constructor(self):
        """Test constructing a block out of endpoints and selectors"""
        block = bc1.BC1Block(*in_endpoints, selectors)
        self.assertEqual(block.tobytes(), block_bytes, 'incorrect block bytes')
        self.assertEqual(block.selectors, selectors, 'incorrect selectors')
        self.assertEqual(block.endpoints, out_endpoints, 'incorrect endpoints')
        self.assertFalse(block.is_3color, 'incorrect color mode')

    def test_frombytes(self):
        """Test constructing a block out of raw data"""
        block = bc1.BC1Block.frombytes(block_bytes)
        self.assertEqual(block.tobytes(), block_bytes, 'incorrect block bytes')
        self.assertEqual(block.selectors, selectors, 'incorrect selectors')
        self.assertEqual(block.endpoints, out_endpoints, 'incorrect endpoints')
        self.assertFalse(block.is_3color, 'incorrect color mode')

    def test_eq(self):
        block1 = bc1.BC1Block.frombytes(block_bytes)
        block2 = bc1.BC1Block.frombytes(block_bytes)
        self.assertEqual(block1, block2, 'identical blocks not equal')


@parameterized_class(
    ("name", "w", "h", "wb", "hb"), [
        ("8x8", 8, 8, 2, 2),
        ("9x9", 9, 9, 3, 3),
        ("7x7", 7, 7, 2, 2),
        ("7x9", 7, 9, 2, 3)
    ])
class TestBC1Texture(unittest.TestCase):
    def setUp(self):
        self.tex = bc1.BC1Texture(self.w, self.h)
        self.size = self.wb * self.hb * bc1.BC1Block.size

    def test_size(self):
        """Test size and dimensions of BC1Texture"""
        self.assertEqual(self.tex.size, self.size, 'incorrect texture size')
        self.assertEqual(len(self.tex.tobytes()), self.size, 'incorrect texture size from tobytes')

        self.assertEqual(self.tex.width, self.w, 'incorrect texture width')
        self.assertEqual(self.tex.height, self.h, 'incorrect texture height')
        self.assertEqual(self.tex.dimensions, (self.w, self.h), 'incorrect texture dimensions')

        self.assertEqual(self.tex.width_blocks, self.wb, 'incorrect texture width_blocks')
        self.assertEqual(self.tex.height_blocks, self.hb, 'incorrect texture width_blocks')
        self.assertEqual(self.tex.dimensions_blocks, (self.wb, self.hb), 'incorrect texture dimensions_blocks')

    def test_blocks(self):
        """Test getting and setting blocks to BC1Texture"""
        blocks = [[bc1.BC1Block.frombytes(bytes([x, y] + [0] * 6)) for x in range(self.wb)] for y in range(self.hb)]
        for x in range(self.wb):
            for y in range(self.hb):
                self.tex[x, y] = blocks[y][x]

        b = self.tex.tobytes()
        for x in range(self.wb):
            for y in range(self.hb):
                index = (x + (y * self.wb)) * bc1.BC1Block.size
                tb = self.tex[x, y]
                fb = bc1.BC1Block.frombytes(b[index:index + bc1.BC1Block.size])
                self.assertEqual(tb, blocks[y][x], 'incorrect block read from texture')
                self.assertEqual(fb, blocks[y][x], 'incorrect block read from texture bytes')

        self.assertEqual(self.tex[-1, -1], self.tex[self.wb - 1, self.hb - 1], 'incorrect negative subscripting')

    def test_buffer(self):
        """Test the buffer protocol of BC1Texture"""
        mv = memoryview(self.tex)

        self.assertFalse(mv.readonly, 'buffer is readonly')
        self.assertTrue(mv.c_contiguous, 'buffer is not contiguous')
        self.assertEqual(mv.ndim, 1, 'buffer is multidimensional')
        self.assertEqual(mv.nbytes, self.size, 'buffer is the wrong size')
        self.assertEqual(mv.format, 'B', 'buffer has the wrong format')

        data = block_bytes * self.wb * self.hb
        mv[:] = data
        self.assertEqual(mv.tobytes(), data, 'incorrect buffer data')


def get_class_name_blocks(cls, num, params_dict):
    return "%s%s" % (cls.__name__, params_dict['color_mode'].name,)


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


if __name__ == '__main__':
    nose.main()
