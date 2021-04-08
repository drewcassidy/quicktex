import unittest
import nose
import os.path
from tests.images import image_path
from quicktex import RawTexture
from PIL import Image


class TestRawTexture(unittest.TestCase):
    boilerplate = Image.open(os.path.join(image_path, 'Boilerplate.png'))
    boilerplate_bytes = boilerplate.tobytes('raw', 'RGBX')
    width, height = boilerplate.size
    size = width * height * 4

    def setUp(self):
        self.tex = RawTexture(self.width, self.height)

    def test_size(self):
        """Test byte size and image dimensions"""
        self.assertEqual(self.tex.nbytes, self.size, "incorrect texture byte size")
        self.assertEqual(self.tex.width, self.width, "incorrect texture width")
        self.assertEqual(self.tex.height, self.height, "incorrect texture height")
        self.assertEqual(self.tex.size, (self.width, self.height), "incorrect texture dimensions")

    def test_pixels(self):
        """Test getting and setting pixel values"""
        color1 = (69, 13, 12, 0)  # totally random color
        color2 = (19, 142, 93, 44)

        self.tex[0, 0] = color1
        self.tex[-1, -1] = color2
        data = self.tex.tobytes()

        self.assertEqual(self.tex[0, 0], color1)
        self.assertEqual(self.tex[-1, -1], color2)
        self.assertEqual(tuple(data[0:4]), color1)
        self.assertEqual(tuple(data[-4:]), color2)

        with self.assertRaises(IndexError):
            thing = self.tex[self.width, self.height]
        with self.assertRaises(IndexError):
            thing = self.tex[-1 - self.width, -1 - self.height]

    def test_buffer(self):
        """Test the Buffer protocol implementation for RawTexture"""
        mv = memoryview(self.tex)

        self.assertFalse(mv.readonly, 'buffer is readonly')
        self.assertTrue(mv.c_contiguous, 'buffer is not contiguous')
        self.assertEqual(mv.nbytes, self.size, 'buffer is the wrong size')
        self.assertEqual(mv.format, 'B', 'buffer has the wrong format')

        mv[:] = self.boilerplate_bytes
        self.assertEqual(mv.tobytes(), self.boilerplate_bytes, 'incorrect buffer data')

    def test_frombytes(self):
        """Test the frombytes factory function"""
        bytetex = RawTexture.frombytes(self.boilerplate_bytes, *self.boilerplate.size)
        self.assertEqual(self.boilerplate_bytes, bytetex.tobytes(), 'Incorrect bytes after writing to buffer')


if __name__ == '__main__':
    nose.main()
