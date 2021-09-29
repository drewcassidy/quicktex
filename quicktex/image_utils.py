"""Various utilities for working with Pillow images"""

from PIL import Image
from typing import List, Tuple, Optional
import math


def mip_sizes(dimensions: Tuple[int, int], mip_count: Optional[int] = None) -> List[Tuple[int, int]]:
    """
    Create a chain of mipmap sizes for a given source source size, where each source is half the size of the one before.
    Note that the division by 2 rounds down. So a 63x63 texture has as its next lowest mipmap level 31x31. And so on.

    See the `OpenGL wiki page on mipmaps <https://www.khronos.org/opengl/wiki/Texture#Mip_maps>`_ for more info.

    :param dimensions: Size of the source source in pixels
    :param mip_count: Number of mipmap sizes to generate. By default, generate until the last mip level is 1x1.
        Resulting mip chain will be smaller if a 1x1 mip level is reached before this value.
    :return: A list of 2-tuples representing the dimensions of each mip level, including ``dimensions`` at element 0.
    """
    assert all([dim > 0 for dim in dimensions]), "Invalid source dimensions"
    if not mip_count:
        mip_count = math.ceil(math.log2(max(dimensions)) + 1)  # maximum possible number of mips for a given source

    assert mip_count > 0, "mip_count must be greater than 0"

    chain = []

    for mip in range(mip_count):
        chain.append(dimensions)
        if all([dim == 1 for dim in dimensions]):
            break  # we've reached a 1x1 mip and can get no smaller
        dimensions = tuple([max(dim // 2, 1) for dim in dimensions])

    return chain


def resize_no_premultiply(image: Image.Image, size: Tuple[int, int]) -> Image.Image:
    """
    Resize an image without premulitplying the alpha. Required due to a quick in Pillow

    :param image: Image to resize
    :param size: Size to resize to
    :return: The resized image
    """
    if image.mode == 'RGBA':
        rgb = image.convert('RGB').resize(size, Image.BILINEAR)
        a = image.getchannel('A').resize(size, Image.BILINEAR)
        rgb.putalpha(a)
        return rgb
    else:
        return image.resize(size, Image.BILINEAR)
