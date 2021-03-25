"""Various utilities for working with Pillow images"""

from PIL import Image
import typing
import math


def pad(image: Image.Image, block_size=(4, 4)) -> Image.Image:
    """
    Pad an image to be divisible by a specific block size. The input image is repeated into the unused areas so that bilinar filtering works correctly.

    :param image: Input image to add padding to. This will not be modified.
    :param block_size: The size of a single block that the output must be divisible by.
    :return: A new image with the specified padding added.
    """

    assert all([dim > 0 for dim in block_size]), "Invalid block size"

    padded_size = tuple([
        math.ceil(i_dim / b_dim) * b_dim
        for i_dim in image.size
        for b_dim in block_size
    ])

    if padded_size == image.size:
        # no padding is necessary
        return image

    output = Image.new(image.mode, padded_size)
    for x in range(math.ceil(padded_size[0] / image.width)):
        for y in range(math.ceil(padded_size[1] / image.height)):
            output.paste(image, (x * image.width, y * image.height))

    return output


def mip_sizes(size: typing.Tuple[int, int], mip_count: typing.Optional[int] = None) -> typing.List[typing.Tuple[int, int]]:
    """
    Create a chain of mipmap sizes for a given source image size, where each image is half the size of the one before.
    Note that the division by 2 rounds down. So a 63x63 texture has as its next lowest mipmap level 31x31. And so on.

    See the `OpenGL wiki page on mipmaps <https://www.khronos.org/opengl/wiki/Texture#Mip_maps>`_ for more info.

    :param size: Size of the source image
    :param mip_count: Number of mipmap sizes to generate. By default, generate until the last mip level is 1x1.
        Resulting mip chain will be smaller if a 1x1 mip level is reached before this value.
    :return: A list of 2-tuples representing the size of each mip level, including ``size`` at element 0.
    """
    assert all([dim > 0 for dim in size]), "Invalid image size"
    if not mip_count:
        mip_count = math.ceil(math.log2(max(size)))  # maximum possible number of mips for a given image

    assert mip_count > 0, "mip_count must be greater than 0"

    chain = []

    for mip in range(mip_count):
        chain.append(size)
        size = tuple([max(dim // 2, 1) for dim in size])

        if all([dim == 1 for dim in size]):
            break  # we've reached a 1x1 mip and can get no smaller

    return chain
