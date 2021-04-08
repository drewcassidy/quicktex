import unittest
import nose
from parameterized import parameterized, parameterized_class
from quicktex.s3tc.bc4 import BC4Block, BC4Texture, BC4Encoder, BC4Decoder
from tests.images import BC4Blocks
from PIL import Image, ImageChops

if __name__ == '__main__':
    nose.main()


class TestBC4Block(unittest.TestCase):
    """Tests for the BC1Block class"""
    block_bytes = b'\xF0\x10\x88\x86\x68\xAC\xCF\xFA'
    selectors = [[0, 1, 2, 3]] * 2 + [[4, 5, 6, 7]] * 2
    endpoints = (240, 16)

    def test_size(self):
        """Test the size and dimensions of BC4Block"""
        self.assertEqual(BC4Block.nbytes, 8, 'incorrect block size')
        self.assertEqual(BC4Block.width, 4, 'incorrect block width')
        self.assertEqual(BC4Block.height, 4, 'incorrect block width')
        self.assertEqual(BC4Block.size, (4, 4), 'incorrect block dimensions')

    def test_buffer(self):
        """Test the buffer protocol of BC4Block"""
        block = BC4Block()
        mv = memoryview(block)

        self.assertFalse(mv.readonly, 'buffer is readonly')
        self.assertTrue(mv.c_contiguous, 'buffer is not contiguous')
        self.assertEqual(mv.ndim, 1, 'buffer is multidimensional')
        self.assertEqual(mv.nbytes, BC4Block.nbytes, 'buffer is the wrong size')
        self.assertEqual(mv.format, 'B', 'buffer has the wrong format')

        mv[:] = self.block_bytes
        self.assertEqual(mv.tobytes(), self.block_bytes, 'incorrect buffer data')

    def test_constructor(self):
        """Test constructing a block out of endpoints and selectors"""
        block = BC4Block(*self.endpoints, self.selectors)
        self.assertEqual(block.tobytes(), self.block_bytes, 'incorrect block bytes')
        self.assertEqual(block.selectors, self.selectors, 'incorrect selectors')
        self.assertEqual(block.endpoints, self.endpoints, 'incorrect endpoints')

    def test_frombytes(self):
        """Test constructing a block out of raw data"""
        block = BC4Block.frombytes(self.block_bytes)
        self.assertEqual(block.tobytes(), self.block_bytes, 'incorrect block bytes')
        self.assertEqual(block.selectors, self.selectors, 'incorrect selectors')
        self.assertEqual(block.endpoints, self.endpoints, 'incorrect endpoints')

    def test_eq(self):
        """Test equality between two identical blocks"""
        block1 = BC4Block.frombytes(self.block_bytes)
        block2 = BC4Block.frombytes(self.block_bytes)
        self.assertEqual(block1, block2, 'identical blocks not equal')

    def test_values_6(self):
        """Test values of a 6-value block"""
        block = BC4Block(8, 248, [[0] * 4] * 4)

        self.assertEqual(block.values, [8, 248, 56, 104, 152, 200, 0, 255], 'incorrect values')
        self.assertTrue(block.is_6value, 'incorrect is_6value')

    def test_values_8(self):
        """Test values of an 8-value block"""
        block = BC4Block(240, 16, [[0] * 4] * 4)

        self.assertEqual(block.values, [240, 16, 208, 176, 144, 112, 80, 48], 'incorrect values')
        self.assertFalse(block.is_6value, 'incorrect is_6value')


@parameterized_class(
    ("name", "w", "h", "wb", "hb"), [
        ("8x8", 8, 8, 2, 2),
        ("9x9", 9, 9, 3, 3),
        ("7x7", 7, 7, 2, 2),
        ("7x9", 7, 9, 2, 3)
    ])
class TestBC4Texture(unittest.TestCase):
    def setUp(self):
        self.tex = BC4Texture(self.w, self.h)
        self.nbytes = self.wb * self.hb * BC4Block.nbytes

    def test_size(self):
        """Test size of BC4Texture in bytes"""
        self.assertEqual(self.tex.nbytes, self.nbytes, 'incorrect texture size')
        self.assertEqual(len(self.tex.tobytes()), self.nbytes, 'incorrect texture size from tobytes')

    def test_dimensions(self):
        """Test dimensions of BC4Texture in pixels"""
        self.assertEqual(self.tex.width, self.w, 'incorrect texture width')
        self.assertEqual(self.tex.height, self.h, 'incorrect texture height')
        self.assertEqual(self.tex.size, (self.w, self.h), 'incorrect texture dimensions')

    def test_dimensions_blocks(self):
        """Test dimensions of BC4Texture in blocks"""
        self.assertEqual(self.tex.width_blocks, self.wb, 'incorrect texture width_blocks')
        self.assertEqual(self.tex.height_blocks, self.hb, 'incorrect texture width_blocks')
        self.assertEqual(self.tex.size_blocks, (self.wb, self.hb), 'incorrect texture dimensions_blocks')

    def test_blocks(self):
        """Test getting and setting blocks to BC4Texture"""
        blocks = [[BC4Block.frombytes(bytes([x, y] + [0] * 6)) for x in range(self.wb)] for y in range(self.hb)]
        for x in range(self.wb):
            for y in range(self.hb):
                self.tex[x, y] = blocks[y][x]

        b = self.tex.tobytes()
        for x in range(self.wb):
            for y in range(self.hb):
                index = (x + (y * self.wb)) * BC4Block.nbytes
                tb = self.tex[x, y]
                fb = BC4Block.frombytes(b[index:index + BC4Block.nbytes])
                self.assertEqual(tb, blocks[y][x], 'incorrect block read from texture')
                self.assertEqual(fb, blocks[y][x], 'incorrect block read from texture bytes')

        self.assertEqual(self.tex[-1, -1], self.tex[self.wb - 1, self.hb - 1], 'incorrect negative subscripting')

        with self.assertRaises(IndexError):
            thing = self.tex[self.wb, self.hb]
        with self.assertRaises(IndexError):
            thing = self.tex[-1 - self.wb, -1 - self.hb]

    def test_buffer(self):
        """Test the buffer protocol of BC4Texture"""
        mv = memoryview(self.tex)

        self.assertFalse(mv.readonly, 'buffer is readonly')
        self.assertTrue(mv.c_contiguous, 'buffer is not contiguous')
        self.assertEqual(mv.nbytes, self.nbytes, 'buffer is the wrong size')
        self.assertEqual(mv.format, 'B', 'buffer has the wrong format')

        data = b'\xF0\x10\x88\x86\x68\xAC\xCF\xFA' * self.wb * self.hb
        mv[:] = data
        self.assertEqual(mv.tobytes(), data, 'incorrect buffer data')


class TestBC4Encoder(unittest.TestCase):
    """Test BC4Encoder"""

    # 6-value blocks are not yet supported by the encoder so we only run one test
    @classmethod
    def setUpClass(cls):
        cls.bc4_encoder = BC4Encoder(0)

    def test_block(self):
        """Test encoder output with 8 value test block"""
        out_tex = self.bc4_encoder.encode(BC4Blocks.eight_value.texture)

        self.assertEqual(out_tex.size_blocks, (1, 1), 'encoded texture has multiple blocks')

        out_block = out_tex[0, 0]

        self.assertFalse(out_block.is_6value, 'returned 6value mode')
        self.assertEqual(out_block, BC4Blocks.eight_value.block, 'encoded block is incorrect')


class TestBC4Decoder(unittest.TestCase):
    """Test BC4Decoder"""

    @classmethod
    def setUpClass(cls):
        cls.bc4_decoder = BC4Decoder(0)

    @parameterized.expand([
        ("8value", BC4Blocks.eight_value.block, BC4Blocks.eight_value.image),
        ("6value", BC4Blocks.six_value.block, BC4Blocks.six_value.image),
    ])
    def test_block(self, _, block, image):
        """Test decoder output for a single block"""
        in_tex = BC4Texture(4, 4)
        in_tex[0, 0] = block
        out_tex = self.bc4_decoder.decode(in_tex)

        self.assertEqual(out_tex.size, (4, 4), 'decoded texture has incorrect dimensions')

        out_img = Image.frombytes('RGBA', (4, 4), out_tex.tobytes())
        img_diff = ImageChops.difference(out_img, image).convert('L')
        img_hist = img_diff.histogram()

        self.assertEqual(16, img_hist[0], 'decoded block is incorrect')
