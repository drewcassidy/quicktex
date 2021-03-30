import unittest
import nose
import quicktex
import images
import tempfile
from PIL import Image


class TestRawTexture(unittest.TestCase):
    def setUp(self):
        self.width = 1024
        self.height = 1024
        self.texture = quicktex.RawTexture(self.width, self.height)
        self.boilerplate = Image.open(images.image_path + '/Boilerplate.png')

    def test_size(self):
        """test byte size and image dimensions"""
        self.assertEqual(self.texture.size, self.width * self.height * 4, "incorrect texture byte size")
        self.assertEqual(self.texture.width, self.width, "incorrect texture width")
        self.assertEqual(self.texture.height, self.height, "incorrect texture height")
        self.assertEqual(self.texture.dimensions, (self.width, self.height), "incorrect texture dimension tuple")

    def test_pixels(self):
        """Test get_ and set_pixel methods for RawTexture"""
        color1 = (69, 13, 12, 0)  # totally random color
        color2 = (19, 142, 93, 44)

        self.texture[0, 0] = color1
        self.texture[-1, -1] = color2
        data = self.texture.tobytes()

        self.assertEqual(self.texture[0, 0], color1)
        self.assertEqual(self.texture[-1, -1], color2)
        self.assertEqual(tuple(data[0:4]), color1)
        self.assertEqual(tuple(data[-4:]), color2)

    def test_buffer(self):
        """Test the Buffer protocol implementation for RawTexture"""
        with tempfile.TemporaryFile('r+b') as fp:
            fp.write(self.boilerplate.tobytes('raw', 'RGBX'))
            fp.seek(0)
            bytes_read = fp.readinto(self.texture)

            self.assertEqual(bytes_read, self.texture.size, 'buffer over/underrun')
        self.assertEqual(self.boilerplate.tobytes('raw', 'RGBX'), self.texture.tobytes(), 'Incorrect bytes after writing to buffer')
        self.assertEqual(bytes(self.texture), self.texture.tobytes(), "Incorrect bytes reading from buffer")

    def test_factory(self):
        """Test the frombytes factory function"""
        bytetex = quicktex.RawTexture.frombytes(self.boilerplate.tobytes('raw', 'RGBX'), *self.boilerplate.size)
        self.assertEqual(self.boilerplate.tobytes('raw', 'RGBX'), bytetex.tobytes(), 'Incorrect bytes after writing to buffer')


if __name__ == '__main__':
    nose.main()
