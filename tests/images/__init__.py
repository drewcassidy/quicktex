from PIL import Image
from quicktex.s3tc.bc1 import BC1Block
from quicktex.s3tc.bc4 import BC4Block
from quicktex import RawTexture
import os.path

image_path = os.path.dirname(os.path.realpath(__file__))


class BC1Blocks:
    class Entry:
        def __init__(self, filename, block):
            path = os.path.join(image_path, 'bc1', filename)
            self.image = Image.open(path).convert('RGBA')
            self.texture = RawTexture.frombytes(self.image.tobytes('raw', 'RGBA'), *self.image.size)
            self.block = block

    greyscale = Entry('greyscale_unpacked.png', BC1Block.frombytes(b'\xFF\xFF\x49\x4A\x78\x78\x78\x78'))
    three_color = Entry('3color_unpacked.png', BC1Block.frombytes(b'\xE0\x07\x00\xF8\x29\x29\x29\x29'))
    three_color_black = Entry('3color_black_unpacked.png', BC1Block.frombytes(b'\xE0\x07\x00\xF8\x27\x27\x27\x27'))


class BC4Blocks:
    class Entry:
        def __init__(self, filename, block):
            path = os.path.join(image_path, 'bc4', filename)
            self.image = Image.open(path).convert('RGBA')
            self.texture = RawTexture.frombytes(self.image.tobytes('raw', 'RGBA'), *self.image.size)
            self.block = block

    six_value = Entry('6value.png', BC4Block(8, 248, [[0, 1, 2, 3]] * 2 + [[4, 5, 6, 7]] * 2))
    eight_value = Entry('8value.png', BC4Block(240, 16, [[0, 1, 2, 3]] * 2 + [[4, 5, 6, 7]] * 2))
