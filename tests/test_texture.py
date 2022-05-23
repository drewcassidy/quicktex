import os.path

import pytest
from PIL import Image

from quicktex import RawTexture
from .images import image_path


class TestRawTexture:
    boilerplate = Image.open(os.path.join(image_path, 'Boilerplate.png'))
    boilerplate_bytes = boilerplate.tobytes('raw', 'RGBX')
    width, height = boilerplate.size
    nbytes = width * height * 4

    def test_size(self):
        """Test byte size and image dimensions"""
        tex = RawTexture(self.width, self.height)
        assert tex.nbytes == self.nbytes
        assert tex.width == self.width
        assert tex.height == self.height
        assert tex.size == (self.width, self.height)

    def test_pixels(self):
        """Test getting and setting pixel values"""
        tex = RawTexture(self.width, self.height)
        color1 = (69, 13, 12, 0)  # totally random color
        color2 = (19, 142, 93, 44)

        tex[0, 0] = color1
        tex[-1, -1] = color2
        data = tex.tobytes()

        assert tex[0, 0] == color1
        assert tex[-1, -1] == color2
        assert tuple(data[0:4]) == color1
        assert tuple(data[-4:]) == color2

        with pytest.raises(IndexError):
            _ = tex[self.width, self.height]
        with pytest.raises(IndexError):
            _ = tex[-1 - self.width, -1 - self.height]

    def test_buffer(self):
        """Test the Buffer protocol implementation for RawTexture"""
        tex = RawTexture(self.width, self.height)
        mv = memoryview(tex)
        mv[:] = self.boilerplate_bytes

        assert not mv.readonly
        assert mv.c_contiguous
        assert mv.nbytes == self.nbytes
        assert mv.format == 'B'
        assert mv.tobytes() == self.boilerplate_bytes
        assert mv.tobytes() == tex.tobytes()

    def test_frombytes(self):
        """Test the frombytes factory function"""
        bytetex = RawTexture.frombytes(self.boilerplate_bytes, *self.boilerplate.size)
        assert self.boilerplate_bytes == bytetex.tobytes()
