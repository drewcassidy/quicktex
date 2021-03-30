import unittest
import nose
import quicktex
import images
import tempfile
from PIL import Image


class TestRawTexture(unittest.TestCase):
    boilerplate = Image.open(images.image_path + '/Boilerplate.png')
    bp_bytes = boilerplate.tobytes('raw', 'RGBX')
    width = boilerplate.width
    height = boilerplate.height

    def setUp(self):
        self.texture = quicktex.RawTexture(self.width, self.height)

    def test_size(self):
        """Test byte size and image dimensions"""
        self.assertEqual(self.texture.size, self.width * self.height * 4, "incorrect texture byte size")
        self.assertEqual(self.texture.width, self.width, "incorrect texture width")
        self.assertEqual(self.texture.height, self.height, "incorrect texture height")
        self.assertEqual(self.texture.dimensions, (self.width, self.height), "incorrect texture dimension tuple")

    def test_pixels(self):
        """Test getting and setting pixel values"""
        color1 = (69, 13, 12, 0)  # totally random color
        color2 = (19, 142, 93, 44)

        self.texture[0, 0] = color1
        self.texture[-1, -1] = color2
        data = self.texture.tobytes()

        self.assertEqual(self.texture[0, 0], color1)
        self.assertEqual(self.texture[-1, -1], color2)
        self.assertEqual(tuple(data[0:4]), color1)
        self.assertEqual(tuple(data[-4:]), color2)
        with self.assertRaises(IndexError):
            thing = self.texture[self.width, self.height]
        with self.assertRaises(IndexError):
            thing = self.texture[-1 - self.width, -1 - self.height]

    def test_buffer(self):
        """Test the Buffer protocol implementation for RawTexture"""
        with tempfile.TemporaryFile('r+b') as fp:
            fp.write(self.bp_bytes)
            fp.seek(0)
            bytes_read = fp.readinto(self.texture)

        self.assertEqual(bytes_read, self.texture.size, 'buffer over/underrun')
        self.assertEqual(self.bp_bytes, self.texture.tobytes(), 'Incorrect bytes after writing to buffer')
        self.assertEqual(self.bp_bytes, bytes(self.texture), "Incorrect bytes after reading from buffer")

    def test_frombytes(self):
        """Test the frombytes factory function"""
        bytetex = quicktex.RawTexture.frombytes(self.bp_bytes, *self.boilerplate.size)
        self.assertEqual(self.bp_bytes, bytetex.tobytes(), 'Incorrect bytes after writing to buffer')


if __name__ == '__main__':
    nose.main()
