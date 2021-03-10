from s3tc import BC1Block
from color import Color
from PIL import Image
import os

image_path = os.path.dirname(os.path.realpath(__file__)) + "/images"

# A block that should always encode greyscale, where every row of pixels is identical, and the left side is lighter than the right side
greyscale = Image.open(image_path + "/blocks/greyscale.png").tobytes("raw", "RGBX")

# A block that should always encode 3-color when available.
# from left to right: red, yellow, yellow, green
three_color = Image.open(image_path + "/blocks/3color.png").tobytes("raw", "RGBX")

# A block that should always encode 3-color with black when available
# from left to right: black, red, yellow, green
three_color_black = Image.open(image_path + "/blocks/3color black.png").tobytes("raw", "RGBX")

bc1_test_blocks = [
    # A block that should always encode greyscale, where every row of pixels is identical, and the left side is lighter than the right side
    {"name": "greyscale",
     "image": Image.open(image_path + "/blocks/greyscale.png").tobytes("raw", "RGBX"),
     "expected": BC1Block(Color(0xFF, 0xFF, 0xFF), Color(0x44, 0x44, 0x44), [[0, 2, 3, 1]] * 4)},

    # A block that should always encode 3-color when available.
    # from left to right: red, yellow, yellow, green
    {"name": "3color",
     "image": Image.open(image_path + "/blocks/3color.png").tobytes("raw", "RGBX"),
     "expected": BC1Block(Color(0x00, 0xFF, 0x00), Color(0xFF, 0x00, 0x00), [[0, 2, 3, 1]] * 4)}
]
