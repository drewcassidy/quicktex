import unittest
import nose
import os.path
from parameterized import parameterized, parameterized_class
import quicktex
from quicktex.s3tc.bc1 import BC1Block, BC1Texture, BC1Encoder, BC1Decoder
from tests.images import BC1Blocks, image_path
from PIL import Image, ImageChops

in_endpoints = ((253, 254, 255), (65, 70, 67))  # has some small changes that should encode the same
out_endpoints = ((255, 255, 255, 255), (66, 69, 66, 255))
selectors = [[0, 2, 3, 1]] * 4
block_bytes = b'\xff\xff\x28\x42\x78\x78\x78\x78'


class TestBC1Block(unittest.TestCase):
    """Tests for the BC1Block class"""

    def test_size(self):
        """Test the size and dimensions of BC1Block"""
        self.assertEqual(BC1Block.nbytes, 8, 'incorrect block size')
        self.assertEqual(BC1Block.width, 4, 'incorrect block width')
        self.assertEqual(BC1Block.height, 4, 'incorrect block width')
        self.assertEqual(BC1Block.size, (4, 4), 'incorrect block dimensions')

    def test_buffer(self):
        """Test the buffer protocol of BC1Block"""
        block = BC1Block()
        mv = memoryview(block)

        self.assertFalse(mv.readonly, 'buffer is readonly')
        self.assertTrue(mv.c_contiguous, 'buffer is not contiguous')
        self.assertEqual(mv.ndim, 1, 'buffer is multidimensional')
        self.assertEqual(mv.nbytes, BC1Block.nbytes, 'buffer is the wrong size')
        self.assertEqual(mv.format, 'B', 'buffer has the wrong format')

        mv[:] = block_bytes
        self.assertEqual(mv.tobytes(), block_bytes, 'incorrect buffer data')

    def test_constructor(self):
        """Test constructing a block out of endpoints and selectors"""
        block = BC1Block(*in_endpoints, selectors)
        self.assertEqual(block.tobytes(), block_bytes, 'incorrect block bytes')
        self.assertEqual(block.selectors, selectors, 'incorrect selectors')
        self.assertEqual(block.endpoints, out_endpoints, 'incorrect endpoints')
        self.assertFalse(block.is_3color, 'incorrect color mode')

    def test_frombytes(self):
        """Test constructing a block out of raw data"""
        block = BC1Block.frombytes(block_bytes)
        self.assertEqual(block.tobytes(), block_bytes, 'incorrect block bytes')
        self.assertEqual(block.selectors, selectors, 'incorrect selectors')
        self.assertEqual(block.endpoints, out_endpoints, 'incorrect endpoints')
        self.assertFalse(block.is_3color, 'incorrect color mode')

    def test_eq(self):
        """Test equality between two identical blocks"""
        block1 = BC1Block.frombytes(block_bytes)
        block2 = BC1Block.frombytes(block_bytes)
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
        self.tex = BC1Texture(self.w, self.h)
        self.nbytes = self.wb * self.hb * BC1Block.nbytes

    def test_size(self):
        """Test size of BC1Texture in bytes"""
        self.assertEqual(self.tex.nbytes, self.nbytes, 'incorrect texture size')
        self.assertEqual(len(self.tex.tobytes()), self.nbytes, 'incorrect texture size from tobytes')

    def test_dimensions(self):
        """Test dimensions of BC1Texture in pixels"""
        self.assertEqual(self.tex.width, self.w, 'incorrect texture width')
        self.assertEqual(self.tex.height, self.h, 'incorrect texture height')
        self.assertEqual(self.tex.size, (self.w, self.h), 'incorrect texture dimensions')

    def test_dimensions_blocks(self):
        """Test dimensions of BC1Texture in blocks"""
        self.assertEqual(self.tex.width_blocks, self.wb, 'incorrect texture width_blocks')
        self.assertEqual(self.tex.height_blocks, self.hb, 'incorrect texture width_blocks')
        self.assertEqual(self.tex.size_blocks, (self.wb, self.hb), 'incorrect texture dimensions_blocks')

    def test_blocks(self):
        """Test getting and setting blocks to BC1Texture"""
        blocks = [[BC1Block.frombytes(bytes([x, y] + [0] * 6)) for x in range(self.wb)] for y in range(self.hb)]
        for x in range(self.wb):
            for y in range(self.hb):
                self.tex[x, y] = blocks[y][x]

        b = self.tex.tobytes()
        for x in range(self.wb):
            for y in range(self.hb):
                index = (x + (y * self.wb)) * BC1Block.nbytes
                tb = self.tex[x, y]
                fb = BC1Block.frombytes(b[index:index + BC1Block.nbytes])
                self.assertEqual(tb, blocks[y][x], 'incorrect block read from texture')
                self.assertEqual(fb, blocks[y][x], 'incorrect block read from texture bytes')

        self.assertEqual(self.tex[-1, -1], self.tex[self.wb - 1, self.hb - 1], 'incorrect negative subscripting')

        with self.assertRaises(IndexError):
            thing = self.tex[self.wb, self.hb]
        with self.assertRaises(IndexError):
            thing = self.tex[-1 - self.wb, -1 - self.hb]

    def test_buffer(self):
        """Test the buffer protocol of BC1Texture"""
        mv = memoryview(self.tex)

        self.assertFalse(mv.readonly, 'buffer is readonly')
        self.assertTrue(mv.c_contiguous, 'buffer is not contiguous')
        self.assertEqual(mv.nbytes, self.nbytes, 'buffer is the wrong size')
        self.assertEqual(mv.format, 'B', 'buffer has the wrong format')

        data = block_bytes * self.wb * self.hb
        mv[:] = data
        self.assertEqual(mv.tobytes(), data, 'incorrect buffer data')


@parameterized_class(
    ("name", "color_mode"), [
        ("4Color", BC1Encoder.ColorMode.FourColor),
        ("3Color", BC1Encoder.ColorMode.ThreeColor),
        ("3Color_Black", BC1Encoder.ColorMode.ThreeColorBlack),
    ])
class TestBC1Encoder(unittest.TestCase):
    """Test BC1Encoder"""

    @classmethod
    def setUpClass(cls):
        cls.bc1_encoder = BC1Encoder(5, cls.color_mode)

    def test_block_4color(self):
        """Test encoder output with 4 color greyscale test block"""
        out_tex = self.bc1_encoder.encode(BC1Blocks.greyscale.texture)

        self.assertEqual(out_tex.size_blocks, (1, 1), 'encoded texture has multiple blocks')

        out_block = out_tex[0, 0]

        self.assertFalse(out_block.is_3color, 'returned 3color mode for greyscale test block')
        self.assertEqual(out_block, BC1Blocks.greyscale.block, 'encoded block is incorrect')

    def test_block_3color(self):
        """Test encoder output with 3 color test block"""
        out_tex = self.bc1_encoder.encode(BC1Blocks.three_color.texture)

        self.assertEqual(out_tex.size_blocks, (1, 1), 'encoded texture has multiple blocks')

        out_block = out_tex[0, 0]

        if self.color_mode != BC1Encoder.ColorMode.FourColor:  # we only care about the selectors if we are in 3 color mode
            self.assertTrue(out_block.is_3color, 'returned 4-color block for 3 color test block')
            self.assertEqual(out_block, BC1Blocks.three_color.block, 'encoded block is incorrect')
        else:
            self.assertFalse(out_block.is_3color, 'returned 3-color block in 4-color mode')

    def test_block_3color_black(self):
        """Test encoder output with 3 color test block with black pixels"""
        out_tex = self.bc1_encoder.encode(BC1Blocks.three_color_black.texture)

        self.assertEqual(out_tex.size_blocks, (1, 1), 'encoded texture has multiple blocks')

        out_block = out_tex[0, 0]
        has_black = 3 in [j for row in out_block.selectors for j in row]

        if self.color_mode == BC1Encoder.ColorMode.ThreeColorBlack:  # we only care about the selectors if we are in 3 color black mode
            self.assertTrue(out_block.is_3color, 'returned 4-color block for 3 color test block with black')
            self.assertTrue(has_black, 'block does not have black pixels as expected')
            self.assertEqual(out_block, BC1Blocks.three_color_black.block, "encoded block is incorrect")
        elif self.color_mode == BC1Encoder.ColorMode.ThreeColor:
            self.assertFalse(has_black and out_block.is_3color, 'returned 3color block with black pixels')
        else:
            self.assertFalse(out_block.is_3color, 'returned 3-color block in 4-color mode')


class TestBC1Decoder(unittest.TestCase):
    """Test BC1Decoder"""

    @classmethod
    def setUpClass(cls):
        cls.bc1_decoder = BC1Decoder()

    @parameterized.expand([
        ("4color", BC1Blocks.greyscale.block, BC1Blocks.greyscale.image),
        ("3color", BC1Blocks.three_color.block, BC1Blocks.three_color.image),
        ("3color_black", BC1Blocks.three_color_black.block, BC1Blocks.three_color_black.image)
    ])
    def test_block(self, _, block, image):
        """Test decoder output for a single block"""
        in_tex = BC1Texture(4, 4)
        in_tex[0, 0] = block
        out_tex = self.bc1_decoder.decode(in_tex)

        self.assertEqual(out_tex.size, (4, 4), 'decoded texture has incorrect dimensions')

        out_img = Image.frombytes('RGBA', (4, 4), out_tex.tobytes())
        img_diff = ImageChops.difference(out_img, image).convert('L')
        img_hist = img_diff.histogram()
        self.assertEqual(16, img_hist[0], 'decoded block is incorrect')


if __name__ == '__main__':
    nose.main()
