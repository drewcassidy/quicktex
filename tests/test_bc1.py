import math

import pytest
from PIL import Image, ImageChops

from quicktex.s3tc.bc1 import BC1Block, BC1Texture, BC1Encoder, BC1Decoder
from .images import BC1Blocks

in_endpoints = ((253, 254, 255), (65, 70, 67))  # has some small changes that should encode the same in 5:6:5
out_endpoints = ((255, 255, 255, 255), (66, 69, 66, 255))
selectors = [[0, 2, 3, 1]] * 4
block_bytes = b'\xff\xff\x28\x42\x78\x78\x78\x78'


class TestBC1Block:
    """Tests for the BC1Block class"""

    def test_size(self):
        """Test the size and dimensions of BC1Block"""
        assert BC1Block.nbytes == 8
        assert BC1Block.width == 4
        assert BC1Block.height == 4
        assert BC1Block.size == (4, 4)

    def test_buffer(self):
        """Test the buffer protocol of BC1Block"""
        block = BC1Block()
        mv = memoryview(block)
        mv[:] = block_bytes

        assert not mv.readonly
        assert mv.c_contiguous
        assert mv.ndim == 1
        assert mv.nbytes == 8
        assert mv.format == 'B'
        assert mv.tobytes() == block_bytes
        assert mv.tobytes() == block.tobytes()

    def test_constructor(self):
        """Test constructing a block out of endpoints and selectors"""
        block = BC1Block(*in_endpoints, selectors)
        assert block.tobytes() == block_bytes
        assert block.selectors == selectors
        assert block.endpoints == out_endpoints
        assert not block.is_3color

    def test_frombytes(self):
        """Test constructing a block out of raw data"""
        block = BC1Block.frombytes(block_bytes)
        assert block.tobytes() == block_bytes
        assert block.selectors == selectors
        assert block.endpoints == out_endpoints
        assert not block.is_3color

    def test_eq(self):
        """Test equality between two identical blocks"""
        block1 = BC1Block.frombytes(block_bytes)
        block2 = BC1Block.frombytes(block_bytes)
        assert block1 == block2


# noinspection PyMethodMayBeStatic
@pytest.mark.parametrize('w', [7, 8, 9])
@pytest.mark.parametrize('h', [7, 8, 9])
class TestBC1Texture:
    def test_dimensions(self, w, h):
        """Test dimensions of BC1Texture in pixels, blocks, and bytes"""
        tex = BC1Texture(w, h)
        wb = math.ceil(w / 4)
        hb = math.ceil(h / 4)

        assert tex.nbytes == BC1Block.nbytes * wb * hb  # block width x block height
        assert len(tex.tobytes()) == tex.nbytes

        assert tex.width == w
        assert tex.height == h
        assert tex.size == (w, h)

        assert tex.width_blocks == wb
        assert tex.height_blocks == hb
        assert tex.size_blocks == (wb, hb)

    def test_blocks(self, w, h):
        """Test getting and setting blocks to BC1Texture"""
        tex = BC1Texture(w, h)

        # generate garbage blocks with the x and y index in the first 2 bytes
        blocks = [
            [BC1Block.frombytes(bytes([x, y] + [0] * 6)) for x in range(tex.width_blocks)]
            for y in range(tex.height_blocks)
        ]
        # assign those blocks to the texture
        for x in range(tex.width_blocks):
            for y in range(tex.height_blocks):
                tex[x, y] = blocks[y][x]

        # get the blocks and analyze
        b = tex.tobytes()
        for x in range(tex.width_blocks):
            for y in range(tex.height_blocks):
                index = (x + (y * tex.width_blocks)) * BC1Block.nbytes
                tb = tex[x, y]
                fb = BC1Block.frombytes(b[index : index + BC1Block.nbytes])
                assert tb == blocks[y][x]
                assert fb == blocks[y][x]

    def text_subscript(self, w, h):
        """Test BC1Texture subscripting for blocks"""
        tex = BC1Texture(w, h)

        # ensure negative wraparound works
        assert tex[-1, -1] == tex[tex.width_blocks - 1, tex.height_blocks - 1]

        with pytest.raises(IndexError):
            _ = tex[tex.width_blocks, tex.height_blocks]
        with pytest.raises(IndexError):
            _ = tex[-1 - tex.width_blocks, -1 - tex.height_blocks]

    def test_buffer(self, w, h):
        """Test the buffer protocol of BC1Texture"""
        tex = BC1Texture(w, h)
        mv = memoryview(tex)

        data = block_bytes * tex.width_blocks * tex.height_blocks
        mv[:] = data

        assert not mv.readonly
        assert mv.c_contiguous
        assert mv.nbytes == tex.nbytes
        assert mv.format == 'B'
        assert mv.tobytes() == data


@pytest.mark.parametrize(
    'color_mode',
    [BC1Encoder.ColorMode.FourColor, BC1Encoder.ColorMode.ThreeColor, BC1Encoder.ColorMode.ThreeColorBlack],
)
class TestBC1Encoder:
    """Test BC1Encoder"""

    def test_block_4color(self, color_mode):
        """Test encoder output with 4 color greyscale test block"""
        encoder = BC1Encoder(color_mode=color_mode)
        out_tex = encoder.encode(BC1Blocks.greyscale.texture)
        out_block = out_tex[0, 0]

        assert out_tex.size_blocks == (1, 1)

        assert not out_block.is_3color
        assert out_block == BC1Blocks.greyscale.block

    def test_block_3color(self, color_mode):
        """Test encoder output with 3 color test block"""
        encoder = BC1Encoder(color_mode=color_mode)
        out_tex = encoder.encode(BC1Blocks.three_color.texture)
        out_block = out_tex[0, 0]

        assert out_tex.size_blocks == (1, 1)

        if encoder.color_mode != BC1Encoder.ColorMode.FourColor:
            # we only care about the selectors if we are in 3 color mode
            assert out_block.is_3color
            assert out_block == BC1Blocks.three_color.block
        else:
            assert not out_block.is_3color

    def test_block_3color_black(self, color_mode):
        """Test encoder output with 3 color test block with black pixels"""
        encoder = BC1Encoder(color_mode=color_mode)
        out_tex = encoder.encode(BC1Blocks.three_color_black.texture)
        out_block = out_tex[0, 0]

        assert out_tex.size_blocks == (1, 1)

        has_black = 3 in [j for row in out_block.selectors for j in row]

        if color_mode == BC1Encoder.ColorMode.ThreeColorBlack:
            # we only care about the selectors if we are in 3 color black mode
            assert out_block.is_3color
            assert has_black
            assert out_block == BC1Blocks.three_color_black.block
        elif color_mode == BC1Encoder.ColorMode.ThreeColor:
            assert not (has_black and out_block.is_3color)
        else:
            assert not out_block.is_3color


@pytest.mark.parametrize('texture', [BC1Blocks.greyscale, BC1Blocks.three_color, BC1Blocks.three_color_black])
class TestBC1Decoder:
    """Test BC1Decoder"""

    def test_block(self, texture):
        """Test decoder output for a single block"""
        block = texture.block
        image = texture.image
        decoder = BC1Decoder()
        in_tex = BC1Texture(4, 4)
        in_tex[0, 0] = block
        out_tex = decoder.decode(in_tex)

        assert out_tex.size == (4, 4)

        out_img = Image.frombytes('RGBA', (4, 4), out_tex.tobytes())
        img_diff = ImageChops.difference(out_img, image).convert('L')
        img_hist = img_diff.histogram()
        assert img_hist[0] == 16
