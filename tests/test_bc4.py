import math

import pytest
from PIL import Image, ImageChops

from quicktex.s3tc.bc4 import BC4Block, BC4Texture, BC4Encoder, BC4Decoder
from .images import BC4Blocks

block_bytes = b'\xF0\x10\x88\x86\x68\xAC\xCF\xFA'
selectors = [[0, 1, 2, 3]] * 2 + [[4, 5, 6, 7]] * 2
endpoints = (240, 16)


class TestBC4Block:
    """Tests for the BC1Block class"""

    def test_size(self):
        """Test the size and dimensions of BC4Block"""
        assert BC4Block.nbytes == 8
        assert BC4Block.width == 4
        assert BC4Block.height == 4
        assert BC4Block.size == (4, 4)

    def test_buffer(self):
        """Test the buffer protocol of BC4Block"""
        block = BC4Block()
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
        block = BC4Block(*endpoints, selectors)
        assert block.tobytes() == block_bytes
        assert block.selectors == selectors
        assert block.endpoints == endpoints

    def test_frombytes(self):
        """Test constructing a block out of raw data"""
        block = BC4Block.frombytes(block_bytes)
        assert block.tobytes() == block_bytes
        assert block.selectors == selectors
        assert block.endpoints == endpoints

    def test_eq(self):
        """Test equality between two identical blocks"""
        block1 = BC4Block.frombytes(block_bytes)
        block2 = BC4Block.frombytes(block_bytes)
        assert block1 == block2

    def test_values_6(self):
        """Test values of a 6-value block"""
        block = BC4Block(8, 248, [[0] * 4] * 4)

        assert block.values == [8, 248, 56, 104, 152, 200, 0, 255]
        assert block.is_6value

    def test_values_8(self):
        """Test values of an 8-value block"""
        block = BC4Block(240, 16, [[0] * 4] * 4)

        assert block.values == [240, 16, 208, 176, 144, 112, 80, 48]
        assert not block.is_6value


# noinspection PyMethodMayBeStatic
@pytest.mark.parametrize('w', [7, 8, 9])
@pytest.mark.parametrize('h', [7, 8, 9])
class TestBC4Texture:
    def test_dimensions(self, w, h):
        """Test dimensions of BC4Texture in pixels, blocks, and bytes"""
        tex = BC4Texture(w, h)
        wb = math.ceil(w / 4)
        hb = math.ceil(h / 4)

        assert tex.nbytes == BC4Block.nbytes * wb * hb  # block width x block height
        assert len(tex.tobytes()) == tex.nbytes

        assert tex.width == w
        assert tex.height == h
        assert tex.size == (w, h)

        assert tex.width_blocks == wb
        assert tex.height_blocks == hb
        assert tex.size_blocks == (wb, hb)

    def test_blocks(self, w, h):
        """Test getting and setting blocks to BC4Texture"""
        tex = BC4Texture(w, h)

        # generate garbage blocks with the x and y index in the first 2 bytes
        blocks = [
            [BC4Block.frombytes(bytes([x, y] + [0] * 6)) for x in range(tex.width_blocks)]
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
                index = (x + (y * tex.width_blocks)) * BC4Block.nbytes
                tb = tex[x, y]
                fb = BC4Block.frombytes(b[index : index + BC4Block.nbytes])
                assert tb == blocks[y][x]
                assert fb == blocks[y][x]

    def text_subscript(self, w, h):
        """Test BC4Texture subscripting for blocks"""
        tex = BC4Texture(w, h)

        # ensure negative wraparound works
        assert tex[-1, -1] == tex[tex.width_blocks - 1, tex.height_blocks - 1]

        with pytest.raises(IndexError):
            _ = tex[tex.width_blocks, tex.height_blocks]
        with pytest.raises(IndexError):
            _ = tex[-1 - tex.width_blocks, -1 - tex.height_blocks]

    def test_buffer(self, w, h):
        """Test the buffer protocol of BC1Texture"""
        tex = BC4Texture(w, h)
        mv = memoryview(tex)

        data = block_bytes * tex.width_blocks * tex.height_blocks
        mv[:] = data

        assert not mv.readonly
        assert mv.c_contiguous
        assert mv.nbytes == tex.nbytes
        assert mv.format == 'B'
        assert mv.tobytes() == data


class TestBC4Encoder:
    """Test BC4Encoder"""

    def test_block(self):
        """Test encoder output with 8 value test block"""
        encoder = BC4Encoder(0)
        out_tex = encoder.encode(BC4Blocks.eight_value.texture)
        out_block = out_tex[0, 0]

        assert out_tex.size_blocks == (1, 1)

        assert not out_block.is_6value
        assert out_block == BC4Blocks.eight_value.block


@pytest.mark.parametrize('texture', [BC4Blocks.eight_value, BC4Blocks.six_value])
class TestBC4Decoder:
    """Test BC4Decoder"""

    def test_block(self, texture):
        """Test decoder output for a single block"""
        block = texture.block
        image = texture.image
        decoder = BC4Decoder(0)
        in_tex = BC4Texture(4, 4)
        in_tex[0, 0] = block
        out_tex = decoder.decode(in_tex)

        assert out_tex.size == (4, 4)

        out_img = Image.frombytes('RGBA', (4, 4), out_tex.tobytes())
        img_diff = ImageChops.difference(out_img, image).convert('L')
        img_hist = img_diff.histogram()

        assert img_hist[0] == 16
